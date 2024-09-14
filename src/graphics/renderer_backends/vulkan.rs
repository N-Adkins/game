use std::{
    collections::{BTreeMap, HashSet},
    ffi::{CStr, CString},
    ptr::{null, null_mut},
    sync::OnceLock,
};

use anyhow::{anyhow, Context, Result};
use ash::{
    khr::{surface, swapchain},
    vk::{self, SwapchainKHR},
    Entry, Instance,
};
use num_traits::clamp;
use tracing::{error, info, trace, warn};

use crate::graphics::Window;

const VALIDATION_LAYERS: &[&str] = &["VK_LAYER_KHRONOS_validation"];
const DEVICE_EXTENSIONS: &[&CStr] = &[vk::KHR_SWAPCHAIN_NAME];
const DYNAMIC_STATES: &[vk::DynamicState] =
    &[vk::DynamicState::VIEWPORT, vk::DynamicState::SCISSOR];

static ENABLE_VALIDATION_LAYERS: OnceLock<bool> = OnceLock::new();

struct VulkanDebug {
    utils: ash::ext::debug_utils::Instance,
    messenger: vk::DebugUtilsMessengerEXT,
}

#[derive(Debug, Default)]
struct QueueFamilies {
    graphics_family: Option<u32>,
    present_family: Option<u32>,
}

impl QueueFamilies {
    fn is_complete(&self) -> bool {
        self.graphics_family.is_some() && self.present_family.is_some()
    }
}

#[allow(unused)]
struct Queues {
    graphics: vk::Queue,
    present: vk::Queue,
}

#[derive(Debug)]
struct SwapChainSupport {
    capabilities: vk::SurfaceCapabilitiesKHR,
    formats: Vec<vk::SurfaceFormatKHR>,
    present_modes: Vec<vk::PresentModeKHR>,
}

#[allow(unused)]
struct SwapChain {
    device: swapchain::Device,
    swapchain: vk::SwapchainKHR,
    images: Vec<vk::Image>,
    image_views: Vec<vk::ImageView>,
    format: vk::Format,
    extent: vk::Extent2D,
}

impl SwapChainSupport {
    fn supported(&self) -> bool {
        !self.formats.is_empty() && !self.present_modes.is_empty()
    }
}

#[allow(unused)]
struct Pipeline {
    layout: vk::PipelineLayout,
    pipeline: vk::Pipeline,
    render_pass: vk::RenderPass,
}

#[allow(unused)]
struct SyncObjects {
    image_available: vk::Semaphore,
    render_finished: vk::Semaphore,
    in_flight: vk::Fence,
}

#[allow(unused)]
pub struct VulkanRenderer {
    entry: ash::Entry, // Basically the vulkan context.
    instance: ash::Instance,
    debug: Option<VulkanDebug>,
    physical_device: vk::PhysicalDevice,
    queue_families: QueueFamilies,
    queues: Queues,
    logical_device: ash::Device,
    surface: surface::Instance,
    surface_khr: vk::SurfaceKHR,
    swapchain: SwapChain,
    pipeline: Pipeline,
    framebuffers: Vec<vk::Framebuffer>,
    command_pool: vk::CommandPool,
    command_buffer: vk::CommandBuffer,
    sync_objects: SyncObjects,
}

impl VulkanRenderer {
    pub fn new(window: &dyn Window) -> Result<Self> {
        let enable_validation_layers: bool = match std::env::var("ENABLE_VALIDATION_LAYERS") {
            Ok(s) => s.parse().unwrap_or(false),
            Err(_) => false,
        };
        _ = ENABLE_VALIDATION_LAYERS.set(enable_validation_layers);

        let entry = unsafe { Entry::load()? };
        let instance = Self::create_instance(&entry, window)?;

        let debug = if *ENABLE_VALIDATION_LAYERS.get().unwrap() {
            Self::check_validation_layers(&entry)?;
            Some(Self::create_debug(&entry, &instance)?)
        } else {
            None
        };

        let surface = Self::create_surface(&entry, &instance);
        let surface_khr = Self::create_surface_khr(&instance, window)?;

        let physical_device = Self::pick_physical_device(&instance, &surface, &surface_khr)?;
        let queue_families =
            Self::get_queue_families(&instance, &physical_device, &surface, &surface_khr);
        let (queues, logical_device) =
            Self::create_logical_device(&instance, &physical_device, &queue_families)?;

        let swapchain = Self::create_swapchain(
            &instance,
            &surface,
            &surface_khr,
            &logical_device,
            &physical_device,
            window,
        )?;

        let pipeline = Self::create_graphics_pipeline(&logical_device, &swapchain)?;
        let framebuffers = Self::create_framebuffers(&logical_device, &swapchain, &pipeline)?;

        let command_pool = Self::create_command_pool(
            &instance,
            &logical_device,
            &physical_device,
            &surface,
            &surface_khr,
        )?;
        let command_buffer = Self::create_command_buffer(&logical_device, &command_pool)?;

        let sync_objects = Self::create_sync_objects(&logical_device)?;

        let this = Self {
            entry,
            instance,
            debug,
            physical_device,
            queue_families,
            queues,
            logical_device,
            surface,
            surface_khr,
            swapchain,
            pipeline,
            framebuffers,
            command_pool,
            command_buffer,
            sync_objects,
        };

        Ok(this)
    }

    fn create_instance(entry: &Entry, window: &dyn Window) -> Result<Instance> {
        trace!("Attempting to create Vulkan instance");

        let app_info = vk::ApplicationInfo {
            api_version: vk::make_api_version(0, 1, 0, 0),
            ..Default::default()
        };

        let extensions = {
            let mut extensions = window.get_requested_extensions();
            if *ENABLE_VALIDATION_LAYERS.get().unwrap() {
                extensions.push(ash::vk::EXT_DEBUG_UTILS_NAME.to_str().unwrap().to_owned());
            }
            extensions
        };

        // We have to have a separate vec owning the CStrings or else we get dumb stuff happening
        // when using the Vulkan API. This is for extensions and validation layers

        let extension_cstrs: Vec<CString> = extensions
            .into_iter()
            .map(|s| CString::new(s).with_context(|| "Failed to parse CString"))
            .collect::<Result<Vec<CString>>>()?;

        let vulkan_extensions: Vec<*const i8> =
            extension_cstrs.iter().map(|s| s.as_ptr()).collect();

        let layer_cstrs: Vec<CString> = VALIDATION_LAYERS
            .iter()
            .map(|s| CString::new(*s).with_context(|| "Failed to parse CString"))
            .collect::<Result<Vec<CString>>>()?;

        let vulkan_layers: Vec<*const i8> = layer_cstrs.iter().map(|s| s.as_ptr()).collect();

        let (layer_count, layer_names, debug_create_info) =
            if *ENABLE_VALIDATION_LAYERS.get().unwrap() {
                trace!("Validation layers enabled, passing debug info to create_instance");
                (
                    vulkan_layers.len() as u32,
                    vulkan_layers.as_ptr(),
                    Some(Self::create_debug_info()),
                )
            } else {
                (0, null(), None)
            };

        let p_next: *const std::ffi::c_void = if let Some(debug_create_info) = debug_create_info {
            &debug_create_info as *const _ as *const _ // idek bruh
        } else {
            null()
        };

        let create_info = vk::InstanceCreateInfo {
            p_application_info: &app_info,
            enabled_extension_count: vulkan_extensions.len() as u32,
            pp_enabled_extension_names: vulkan_extensions.as_ptr(),
            enabled_layer_count: layer_count,
            pp_enabled_layer_names: layer_names,
            p_next,
            ..Default::default()
        };

        let instance = unsafe { entry.create_instance(&create_info, None)? };

        trace!("Created Vulkan instance");

        Ok(instance)
    }

    fn check_validation_layers(entry: &ash::Entry) -> Result<()> {
        trace!("Checking supported Vulkan validation layers");

        let available_layers = unsafe { entry.enumerate_instance_layer_properties()? };
        trace!("Supported validation layers:");
        for layer in available_layers.iter() {
            trace!("{}", layer.layer_name_as_c_str().unwrap().to_str().unwrap());
        }

        for layer in VALIDATION_LAYERS {
            trace!("Checking if layer \"{layer}\" is supported");
            let found: bool = 'block: {
                for available_layer in available_layers.iter() {
                    if available_layer
                        .layer_name_as_c_str()
                        .expect("Vulkan layer name was not null-terminated")
                        .to_str()
                        .expect("Vulkan layer name was not in UTF-8")
                        == *layer
                    {
                        break 'block true;
                    }
                }
                break 'block false;
            };
            if !found {
                error!("Validation layer \"{layer}\" is not supported on this device");
                _ = anyhow!("Not all validation layers are supported"); // anyhow is being weird here
            }
        }

        Ok(())
    }

    fn create_debug_info<'a>() -> vk::DebugUtilsMessengerCreateInfoEXT<'a> {
        vk::DebugUtilsMessengerCreateInfoEXT {
            message_severity: vk::DebugUtilsMessageSeverityFlagsEXT::VERBOSE
                | vk::DebugUtilsMessageSeverityFlagsEXT::INFO
                | vk::DebugUtilsMessageSeverityFlagsEXT::WARNING
                | vk::DebugUtilsMessageSeverityFlagsEXT::ERROR,
            message_type: vk::DebugUtilsMessageTypeFlagsEXT::GENERAL
                | vk::DebugUtilsMessageTypeFlagsEXT::PERFORMANCE
                | vk::DebugUtilsMessageTypeFlagsEXT::VALIDATION,
            pfn_user_callback: Some(Self::vulkan_debug_callback),
            ..Default::default()
        }
    }

    fn create_debug(entry: &ash::Entry, instance: &ash::Instance) -> Result<VulkanDebug> {
        trace!("Attempting to create Vulkan Debug Messenger");
        let utils = ash::ext::debug_utils::Instance::new(entry, instance);
        let create_info = Self::create_debug_info();
        let messenger = unsafe { utils.create_debug_utils_messenger(&create_info, None)? };
        trace!("Created Vulkan Debug Messenger");
        Ok(VulkanDebug { utils, messenger })
    }

    #[no_mangle]
    unsafe extern "system" fn vulkan_debug_callback(
        msg_severity: vk::DebugUtilsMessageSeverityFlagsEXT,
        msg_type: vk::DebugUtilsMessageTypeFlagsEXT,
        callback_data: *const vk::DebugUtilsMessengerCallbackDataEXT,
        _user_data: *mut std::ffi::c_void,
    ) -> vk::Bool32 {
        let vulkan_msg = CStr::from_ptr((*callback_data).p_message);
        let msg = format!("Vulkan [{msg_type:?}]: {}", vulkan_msg.to_str().unwrap());
        match msg_severity {
            vk::DebugUtilsMessageSeverityFlagsEXT::VERBOSE => trace!("{msg}"),
            vk::DebugUtilsMessageSeverityFlagsEXT::INFO => info!("{msg}"),
            vk::DebugUtilsMessageSeverityFlagsEXT::WARNING => warn!("{msg}"),
            vk::DebugUtilsMessageSeverityFlagsEXT::ERROR => error!("{msg}"),
            _ => error!("Invalid Vulkan debug message severity"),
        }
        vk::FALSE
    }

    fn check_device_extensions(
        instance: &ash::Instance,
        device: &vk::PhysicalDevice,
    ) -> Result<bool> {
        let available_extensions =
            unsafe { instance.enumerate_device_extension_properties(*device)? };
        let mut required_extensions: HashSet<&CStr> =
            HashSet::from_iter(DEVICE_EXTENSIONS.iter().cloned());

        for extension in available_extensions {
            required_extensions.remove(extension.extension_name_as_c_str()?);
        }

        Ok(required_extensions.is_empty())
    }

    fn get_swapchain_support(
        surface: &surface::Instance,
        surface_khr: &vk::SurfaceKHR,
        device: &vk::PhysicalDevice,
    ) -> Result<SwapChainSupport> {
        let capabilities =
            unsafe { surface.get_physical_device_surface_capabilities(*device, *surface_khr)? };
        let formats =
            unsafe { surface.get_physical_device_surface_formats(*device, *surface_khr)? };
        let present_modes =
            unsafe { surface.get_physical_device_surface_present_modes(*device, *surface_khr)? };
        Ok(SwapChainSupport {
            capabilities,
            formats,
            present_modes,
        })
    }

    fn rank_device(
        instance: &ash::Instance,
        device: &vk::PhysicalDevice,
        surface: &surface::Instance,
        surface_khr: &vk::SurfaceKHR,
    ) -> Result<usize> {
        let properties = unsafe { instance.get_physical_device_properties(*device) };
        let features = unsafe { instance.get_physical_device_features(*device) };

        let families = Self::get_queue_families(instance, device, surface, surface_khr);
        let swapchain_support = Self::get_swapchain_support(surface, surface_khr, device)?;

        if features.geometry_shader == 0
            || !families.is_complete()
            || !Self::check_device_extensions(instance, device)?
            || !swapchain_support.supported()
        {
            return Ok(0); // Necessary for usage
        }

        let mut score: usize = 0;

        if properties.device_type == vk::PhysicalDeviceType::DISCRETE_GPU {
            score += 1000;
        }

        score += properties.limits.max_image_dimension2_d as usize;

        trace!(
            "Physical Device \"{}\" was scored {score}",
            properties.device_name_as_c_str().unwrap().to_str().unwrap()
        );

        Ok(score)
    }

    fn pick_physical_device(
        instance: &ash::Instance,
        surface: &surface::Instance,
        surface_khr: &vk::SurfaceKHR,
    ) -> Result<vk::PhysicalDevice> {
        trace!("Attempting to pick Vulkan Physical Device");

        let devices = unsafe { instance.enumerate_physical_devices()? };
        let mut candidates: BTreeMap<usize, vk::PhysicalDevice> = BTreeMap::new();

        for device in devices {
            let score = Self::rank_device(instance, &device, surface, surface_khr)?;
            candidates.insert(score, device);
        }

        let device: Option<vk::PhysicalDevice> = {
            if candidates.is_empty() {
                None
            } else if let Some((score, device)) = candidates.first_key_value() {
                if *score == 0 {
                    None
                } else {
                    Some(*device)
                }
            } else {
                None
            }
        };

        if let Some(device) = device {
            let properties = unsafe { instance.get_physical_device_properties(device) };
            trace!(
                "Successfully selected Physical Device \"{}\"",
                properties.device_name_as_c_str().unwrap().to_str().unwrap()
            );
            Ok(device)
        } else {
            error!("Failed to find suitable GPU");
            Err(anyhow!("Vulkan could not find a suitable GPU"))
        }
    }

    fn get_queue_families(
        instance: &ash::Instance,
        device: &vk::PhysicalDevice,
        surface: &surface::Instance,
        surface_khr: &vk::SurfaceKHR,
    ) -> QueueFamilies {
        trace!("Getting queue families");

        let mut families = QueueFamilies {
            ..Default::default()
        };

        let properties = unsafe { instance.get_physical_device_queue_family_properties(*device) };
        for (i, property) in properties.into_iter().enumerate() {
            if property.queue_flags.contains(vk::QueueFlags::GRAPHICS) {
                families.graphics_family = Some(i as u32);
            }
            let present_support: bool = unsafe {
                surface
                    .get_physical_device_surface_support(*device, i as u32, *surface_khr)
                    .unwrap_or(false)
            };
            if present_support {
                families.present_family = Some(i as u32);
            }
            if families.is_complete() {
                break;
            }
        }

        if families.is_complete() {
            trace!("Queue families are complete");
        } else {
            trace!("Queue families are not complete");
        }

        trace!("Successfully got queue families");

        families
    }

    fn create_logical_device(
        instance: &ash::Instance,
        physical_device: &vk::PhysicalDevice,
        queue_families: &QueueFamilies,
    ) -> Result<(Queues, ash::Device)> {
        assert!(queue_families.is_complete());

        trace!("Attempting to create Vulkan logical device");

        let mut unique_families: HashSet<u32> = HashSet::new();
        unique_families.insert(queue_families.graphics_family.unwrap());
        unique_families.insert(queue_families.present_family.unwrap());

        let queue_priorities: f32 = 1.0;
        let queue_infos: Vec<vk::DeviceQueueCreateInfo> = unique_families
            .into_iter()
            .map(|family| vk::DeviceQueueCreateInfo {
                s_type: vk::StructureType::DEVICE_QUEUE_CREATE_INFO,
                queue_family_index: family,
                queue_count: 1,
                p_queue_priorities: &queue_priorities,
                ..Default::default()
            })
            .collect();

        let extension_names: Vec<*const i8> =
            DEVICE_EXTENSIONS.iter().map(|cstr| cstr.as_ptr()).collect();

        let device_features = vk::PhysicalDeviceFeatures {
            ..Default::default()
        };
        let device_info = vk::DeviceCreateInfo {
            s_type: vk::StructureType::DEVICE_CREATE_INFO,
            p_queue_create_infos: queue_infos.as_ptr(),
            queue_create_info_count: queue_infos.len() as u32,
            p_enabled_features: &device_features,
            pp_enabled_extension_names: extension_names.as_ptr(),
            enabled_extension_count: extension_names.len() as u32,
            ..Default::default()
        };
        let device = unsafe { instance.create_device(*physical_device, &device_info, None)? };

        let queues = unsafe {
            let graphics = device.get_device_queue(queue_families.graphics_family.unwrap(), 0);
            let present = device.get_device_queue(queue_families.present_family.unwrap(), 0);
            Queues { graphics, present }
        };

        trace!("Created Vulkan logical device");

        Ok((queues, device))
    }

    fn create_surface(entry: &ash::Entry, instance: &ash::Instance) -> surface::Instance {
        surface::Instance::new(entry, instance)
    }

    fn create_surface_khr(instance: &ash::Instance, window: &dyn Window) -> Result<vk::SurfaceKHR> {
        window.create_vulkan_surface(instance.handle())
    }

    fn choose_swapchain_format(formats: &[vk::SurfaceFormatKHR]) -> Result<vk::SurfaceFormatKHR> {
        for format in formats.iter() {
            if format.format == vk::Format::B8G8R8A8_SRGB
                && format.color_space == vk::ColorSpaceKHR::SRGB_NONLINEAR
            {
                return Ok(*format);
            }
        }

        // If no ideal formats, just pick first one
        if !formats.is_empty() {
            return Ok(*formats.first().unwrap());
        }

        // No formats at all
        Err(anyhow!("No valid swapchain formats were provided"))
    }

    fn choose_swapchain_present_mode(modes: &[vk::PresentModeKHR]) -> vk::PresentModeKHR {
        for mode in modes {
            if *mode == vk::PresentModeKHR::MAILBOX {
                return *mode;
            }
        }

        // Guaranteed to be available
        vk::PresentModeKHR::FIFO
    }

    fn choose_swapchain_extent(
        capabilities: &vk::SurfaceCapabilitiesKHR,
        window: &dyn Window,
    ) -> vk::Extent2D {
        if capabilities.current_extent.width != u32::MAX {
            return capabilities.current_extent;
        }
        let (window_width, window_height) = window.get_size();
        let width = clamp(
            window_width,
            capabilities.min_image_extent.width,
            capabilities.max_image_extent.width,
        );
        let height = clamp(
            window_height,
            capabilities.min_image_extent.height,
            capabilities.max_image_extent.height,
        );
        vk::Extent2D { width, height }
    }

    fn create_swapchain(
        instance: &ash::Instance,
        surface: &surface::Instance,
        surface_khr: &vk::SurfaceKHR,
        logical_device: &ash::Device,
        physical_device: &vk::PhysicalDevice,
        window: &dyn Window,
    ) -> Result<SwapChain> {
        trace!("Attempting to create Vulkan swapchain");

        let swapchain_device = swapchain::Device::new(instance, logical_device);

        let support = Self::get_swapchain_support(surface, surface_khr, physical_device)?;
        let format = Self::choose_swapchain_format(&support.formats)?;
        let present_mode = Self::choose_swapchain_present_mode(&support.present_modes);
        let extent = Self::choose_swapchain_extent(&support.capabilities, window);

        let image_count = if support.capabilities.max_image_count > 0
            && support.capabilities.min_image_count + 1 > support.capabilities.max_image_count
        {
            support.capabilities.max_image_count
        } else {
            support.capabilities.min_image_count + 1
        };

        let families = Self::get_queue_families(instance, physical_device, surface, surface_khr);
        let indices: Vec<u32> = vec![
            families.graphics_family.unwrap(),
            families.present_family.unwrap(),
        ];

        let (image_sharing_mode, queue_family_index_count, p_queue_family_indices): (
            vk::SharingMode,
            u32,
            *const u32,
        ) = if families.graphics_family.unwrap() != families.present_family.unwrap() {
            (vk::SharingMode::CONCURRENT, 2, indices.as_ptr())
        } else {
            (vk::SharingMode::EXCLUSIVE, 0, null())
        };

        let create_info = vk::SwapchainCreateInfoKHR {
            s_type: vk::StructureType::SWAPCHAIN_CREATE_INFO_KHR,
            surface: *surface_khr,
            min_image_count: image_count,
            image_format: format.format,
            image_color_space: format.color_space,
            image_extent: extent,
            image_array_layers: 1,
            image_usage: vk::ImageUsageFlags::COLOR_ATTACHMENT,
            pre_transform: support.capabilities.current_transform,
            composite_alpha: vk::CompositeAlphaFlagsKHR::OPAQUE,
            clipped: vk::TRUE,
            old_swapchain: SwapchainKHR::null(),
            present_mode,
            image_sharing_mode,
            queue_family_index_count,
            p_queue_family_indices,
            ..Default::default()
        };

        let swapchain = unsafe { swapchain_device.create_swapchain(&create_info, None)? };

        let images = unsafe { swapchain_device.get_swapchain_images(swapchain)? };

        let image_views: Vec<vk::ImageView> = images
            .iter()
            .map(|image| {
                let info = vk::ImageViewCreateInfo {
                    s_type: vk::StructureType::IMAGE_VIEW_CREATE_INFO,
                    image: *image,
                    view_type: vk::ImageViewType::TYPE_2D,
                    format: format.format,
                    components: vk::ComponentMapping {
                        r: vk::ComponentSwizzle::IDENTITY,
                        g: vk::ComponentSwizzle::IDENTITY,
                        b: vk::ComponentSwizzle::IDENTITY,
                        a: vk::ComponentSwizzle::IDENTITY,
                    },
                    subresource_range: vk::ImageSubresourceRange {
                        aspect_mask: vk::ImageAspectFlags::COLOR,
                        base_mip_level: 0,
                        level_count: 1,
                        base_array_layer: 0,
                        layer_count: 1,
                    },
                    ..Default::default()
                };
                let view: Result<vk::ImageView> =
                    match unsafe { logical_device.create_image_view(&info, None) } {
                        Ok(view) => Ok(view),
                        Err(e) => Err(anyhow!("Vulkan image view creation error: {e}")),
                    };
                view
            })
            .collect::<Result<Vec<vk::ImageView>>>()?;

        let swapchain = SwapChain {
            device: swapchain_device,
            swapchain,
            images,
            image_views,
            format: format.format,
            extent,
        };

        trace!("Created Vulkan swapchain");

        Ok(swapchain)
    }

    fn create_shader_module(logical_device: &ash::Device, code: &[u8]) -> Result<vk::ShaderModule> {
        let info = vk::ShaderModuleCreateInfo {
            s_type: vk::StructureType::SHADER_MODULE_CREATE_INFO,
            code_size: code.len(),
            p_code: code.as_ptr() as *const u32,
            ..Default::default()
        };

        let shader_module = unsafe { logical_device.create_shader_module(&info, None)? };

        Ok(shader_module)
    }

    fn create_render_pass(
        logical_device: &ash::Device,
        swapchain: &SwapChain,
    ) -> Result<vk::RenderPass> {
        let attachment = vk::AttachmentDescription {
            format: swapchain.format,
            samples: vk::SampleCountFlags::TYPE_1,
            load_op: vk::AttachmentLoadOp::CLEAR,
            store_op: vk::AttachmentStoreOp::STORE,
            stencil_load_op: vk::AttachmentLoadOp::DONT_CARE,
            stencil_store_op: vk::AttachmentStoreOp::DONT_CARE,
            initial_layout: vk::ImageLayout::UNDEFINED,
            final_layout: vk::ImageLayout::PRESENT_SRC_KHR,
            ..Default::default()
        };

        let attachment_ref = vk::AttachmentReference {
            attachment: 0,
            layout: vk::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,
        };

        let subpass = vk::SubpassDescription {
            pipeline_bind_point: vk::PipelineBindPoint::GRAPHICS,
            color_attachment_count: 1,
            p_color_attachments: &attachment_ref,
            ..Default::default()
        };

        let dependency = vk::SubpassDependency {
            src_subpass: vk::SUBPASS_EXTERNAL,
            dst_subpass: 0,
            src_stage_mask: vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT,
            src_access_mask: vk::AccessFlags::empty(),
            dst_stage_mask: vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT,
            dst_access_mask: vk::AccessFlags::COLOR_ATTACHMENT_WRITE,
            ..Default::default()
        };

        let info = vk::RenderPassCreateInfo {
            s_type: vk::StructureType::RENDER_PASS_CREATE_INFO,
            attachment_count: 1,
            p_attachments: &attachment,
            subpass_count: 1,
            p_subpasses: &subpass,
            dependency_count: 1,
            p_dependencies: &dependency,
            ..Default::default()
        };

        let render_pass = unsafe { logical_device.create_render_pass(&info, None)? };

        Ok(render_pass)
    }

    fn create_graphics_pipeline(
        logical_device: &ash::Device,
        swapchain: &SwapChain,
    ) -> Result<Pipeline> {
        let vert_code = include_bytes!("shaders/vert.spv");
        let frag_code = include_bytes!("shaders/frag.spv");

        let vert_module = Self::create_shader_module(logical_device, vert_code)?;
        let frag_module = Self::create_shader_module(logical_device, frag_code)?;

        let vert_info = vk::PipelineShaderStageCreateInfo {
            s_type: vk::StructureType::PIPELINE_SHADER_STAGE_CREATE_INFO,
            stage: vk::ShaderStageFlags::VERTEX,
            module: vert_module,
            p_name: "main".as_ptr() as *const i8,
            ..Default::default()
        };

        let frag_info = vk::PipelineShaderStageCreateInfo {
            s_type: vk::StructureType::PIPELINE_SHADER_STAGE_CREATE_INFO,
            stage: vk::ShaderStageFlags::FRAGMENT,
            module: frag_module,
            p_name: "main".as_ptr() as *const i8,
            ..Default::default()
        };

        let shader_stages = [vert_info, frag_info];

        let dynamic_state_info = vk::PipelineDynamicStateCreateInfo {
            s_type: vk::StructureType::PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            dynamic_state_count: DYNAMIC_STATES.len() as u32,
            p_dynamic_states: DYNAMIC_STATES.as_ptr(),
            ..Default::default()
        };

        let vertex_input_info = vk::PipelineVertexInputStateCreateInfo {
            s_type: vk::StructureType::PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            vertex_binding_description_count: 0,
            p_vertex_binding_descriptions: null(),
            vertex_attribute_description_count: 0,
            p_vertex_attribute_descriptions: null(),
            ..Default::default()
        };

        let assembly_info = vk::PipelineInputAssemblyStateCreateInfo {
            s_type: vk::StructureType::PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            topology: vk::PrimitiveTopology::TRIANGLE_LIST,
            primitive_restart_enable: vk::FALSE,
            ..Default::default()
        };

        let viewport_state_info = vk::PipelineViewportStateCreateInfo {
            s_type: vk::StructureType::PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            viewport_count: 1,
            scissor_count: 1,
            ..Default::default()
        };

        let rasterizer_info = vk::PipelineRasterizationStateCreateInfo {
            s_type: vk::StructureType::PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            depth_clamp_enable: vk::FALSE,
            rasterizer_discard_enable: vk::FALSE,
            polygon_mode: vk::PolygonMode::FILL,
            line_width: 1.0,
            cull_mode: vk::CullModeFlags::BACK,
            front_face: vk::FrontFace::CLOCKWISE,
            depth_bias_enable: vk::FALSE,
            depth_bias_constant_factor: 0.0,
            depth_bias_clamp: 0.0,
            depth_bias_slope_factor: 0.0,
            ..Default::default()
        };

        let multisampling_info = vk::PipelineMultisampleStateCreateInfo {
            s_type: vk::StructureType::PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            sample_shading_enable: vk::FALSE,
            rasterization_samples: vk::SampleCountFlags::TYPE_1,
            min_sample_shading: 1.0,
            p_sample_mask: null(),
            alpha_to_coverage_enable: vk::FALSE,
            alpha_to_one_enable: vk::FALSE,
            ..Default::default()
        };

        let blend_info = vk::PipelineColorBlendAttachmentState {
            color_write_mask: vk::ColorComponentFlags::R
                | vk::ColorComponentFlags::G
                | vk::ColorComponentFlags::B
                | vk::ColorComponentFlags::A,
            blend_enable: vk::FALSE,
            src_color_blend_factor: vk::BlendFactor::ONE,
            dst_color_blend_factor: vk::BlendFactor::ZERO,
            color_blend_op: vk::BlendOp::ADD,
            src_alpha_blend_factor: vk::BlendFactor::ONE,
            dst_alpha_blend_factor: vk::BlendFactor::ZERO,
            alpha_blend_op: vk::BlendOp::ADD,
        };

        let blend_state_info = vk::PipelineColorBlendStateCreateInfo {
            s_type: vk::StructureType::PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            logic_op_enable: vk::FALSE,
            logic_op: vk::LogicOp::COPY,
            attachment_count: 1,
            p_attachments: &blend_info,
            blend_constants: [0.0, 0.0, 0.0, 0.0],
            ..Default::default()
        };

        let layout_info = vk::PipelineLayoutCreateInfo {
            s_type: vk::StructureType::PIPELINE_LAYOUT_CREATE_INFO,
            set_layout_count: 0,
            p_set_layouts: null(),
            push_constant_range_count: 0,
            p_push_constant_ranges: null(),
            ..Default::default()
        };

        let layout = unsafe { logical_device.create_pipeline_layout(&layout_info, None)? };
        let render_pass = Self::create_render_pass(logical_device, swapchain)?;

        let info = vk::GraphicsPipelineCreateInfo {
            s_type: vk::StructureType::GRAPHICS_PIPELINE_CREATE_INFO,
            stage_count: shader_stages.len() as u32,
            p_stages: shader_stages.as_ptr(),
            p_vertex_input_state: &vertex_input_info,
            p_input_assembly_state: &assembly_info,
            p_viewport_state: &viewport_state_info,
            p_rasterization_state: &rasterizer_info,
            p_multisample_state: &multisampling_info,
            p_depth_stencil_state: null(),
            p_color_blend_state: &blend_state_info,
            p_dynamic_state: &dynamic_state_info,
            layout,
            render_pass,
            subpass: 0,
            base_pipeline_handle: vk::Pipeline::null(),
            base_pipeline_index: -1,
            ..Default::default()
        };

        let pipelines = match unsafe {
            logical_device.create_graphics_pipelines(vk::PipelineCache::null(), &[info], None)
        } {
            Ok(pipelines) => pipelines,
            Err(_) => return Err(anyhow!("Failed to create Vulkan pipeline")),
        };

        let pipeline = Pipeline {
            layout,
            pipeline: *pipelines.first().unwrap(),
            render_pass,
        };

        unsafe { logical_device.destroy_shader_module(vert_module, None) };
        unsafe { logical_device.destroy_shader_module(frag_module, None) };

        Ok(pipeline)
    }

    fn create_framebuffers(
        logical_device: &ash::Device,
        swapchain: &SwapChain,
        pipeline: &Pipeline,
    ) -> Result<Vec<vk::Framebuffer>> {
        let framebuffers = swapchain
            .image_views
            .iter()
            .map(|view| {
                let attachments = [*view];

                let info = vk::FramebufferCreateInfo {
                    s_type: vk::StructureType::FRAMEBUFFER_CREATE_INFO,
                    render_pass: pipeline.render_pass,
                    attachment_count: attachments.len() as u32,
                    p_attachments: attachments.as_ptr(),
                    width: swapchain.extent.width,
                    height: swapchain.extent.height,
                    layers: 1,
                    ..Default::default()
                };

                let framebuffer = match unsafe { logical_device.create_framebuffer(&info, None) } {
                    Ok(framebuffer) => framebuffer,
                    Err(_) => return Err(anyhow!("Failed to create framebuffer")),
                };

                Ok(framebuffer)
            })
            .collect::<Result<Vec<vk::Framebuffer>>>()?;

        Ok(framebuffers)
    }

    fn create_command_pool(
        instance: &ash::Instance,
        logical_device: &ash::Device,
        physical_device: &vk::PhysicalDevice,
        surface: &surface::Instance,
        surface_khr: &vk::SurfaceKHR,
    ) -> Result<vk::CommandPool> {
        let indices = Self::get_queue_families(instance, physical_device, surface, surface_khr);

        let info = vk::CommandPoolCreateInfo {
            s_type: vk::StructureType::COMMAND_POOL_CREATE_INFO,
            flags: vk::CommandPoolCreateFlags::RESET_COMMAND_BUFFER,
            queue_family_index: indices.graphics_family.unwrap(),
            ..Default::default()
        };

        let pool = unsafe { logical_device.create_command_pool(&info, None)? };
        Ok(pool)
    }

    fn create_command_buffer(
        logical_device: &ash::Device,
        command_pool: &vk::CommandPool,
    ) -> Result<vk::CommandBuffer> {
        let alloc_info = vk::CommandBufferAllocateInfo {
            s_type: vk::StructureType::COMMAND_BUFFER_ALLOCATE_INFO,
            command_pool: *command_pool,
            level: vk::CommandBufferLevel::PRIMARY,
            command_buffer_count: 1,
            ..Default::default()
        };

        let command_buffer = unsafe { logical_device.allocate_command_buffers(&alloc_info)? };

        Ok(*command_buffer.first().unwrap())
    }

    fn create_sync_objects(logical_device: &ash::Device) -> Result<SyncObjects> {
        let semaphore_info = vk::SemaphoreCreateInfo {
            s_type: vk::StructureType::SEMAPHORE_CREATE_INFO,
            ..Default::default()
        };

        let fence_info = vk::FenceCreateInfo {
            s_type: vk::StructureType::FENCE_CREATE_INFO,
            flags: vk::FenceCreateFlags::SIGNALED,
            ..Default::default()
        };

        unsafe {
            let image_available = logical_device.create_semaphore(&semaphore_info, None)?;
            let render_finished = logical_device.create_semaphore(&semaphore_info, None)?;
            let in_flight = logical_device.create_fence(&fence_info, None)?;
            let sync_objects = SyncObjects {
                image_available,
                render_finished,
                in_flight,
            };
            Ok(sync_objects)
        }
    }

    fn record_command_buffer(
        &self,
        command_buffer: vk::CommandBuffer,
        image_index: u32,
    ) -> Result<()> {
        let begin_info = vk::CommandBufferBeginInfo {
            s_type: vk::StructureType::COMMAND_BUFFER_BEGIN_INFO,
            flags: vk::CommandBufferUsageFlags::empty(),
            p_inheritance_info: null(),
            ..Default::default()
        };

        match unsafe {
            self.logical_device
                .begin_command_buffer(command_buffer, &begin_info)
        } {
            Ok(()) => {}
            Err(_) => return Err(anyhow!("Failed to create Vulkan command buffer")),
        }

        let clear_color = vk::ClearValue {
            color: vk::ClearColorValue {
                float32: [0.0, 0.0, 0.0, 1.0],
            },
        };

        let pass_info = vk::RenderPassBeginInfo {
            s_type: vk::StructureType::RENDER_PASS_BEGIN_INFO,
            render_pass: self.pipeline.render_pass,
            framebuffer: *self.framebuffers.get(image_index as usize).unwrap(),
            render_area: vk::Rect2D {
                offset: vk::Offset2D { x: 0, y: 0 },
                extent: self.swapchain.extent,
            },
            clear_value_count: 1,
            p_clear_values: &clear_color,
            ..Default::default()
        };

        unsafe {
            self.logical_device.cmd_begin_render_pass(
                command_buffer,
                &pass_info,
                vk::SubpassContents::INLINE,
            )
        };
        unsafe {
            self.logical_device.cmd_bind_pipeline(
                command_buffer,
                vk::PipelineBindPoint::GRAPHICS,
                self.pipeline.pipeline,
            )
        };

        let viewport = vk::Viewport {
            x: 0.0,
            y: 0.0,
            width: self.swapchain.extent.width as f32,
            height: self.swapchain.extent.height as f32,
            min_depth: 0.0,
            max_depth: 1.0,
        };
        unsafe {
            self.logical_device
                .cmd_set_viewport(command_buffer, 0, &[viewport])
        };

        let scissor = vk::Rect2D {
            offset: vk::Offset2D { x: 0, y: 0 },
            extent: self.swapchain.extent,
        };
        unsafe {
            self.logical_device
                .cmd_set_scissor(command_buffer, 0, &[scissor]);
            self.logical_device.cmd_draw(command_buffer, 3, 1, 0, 0);
            self.logical_device.cmd_end_render_pass(command_buffer);
            self.logical_device.end_command_buffer(command_buffer)?;
        };

        Ok(())
    }

    pub fn draw_frame(&self) -> Result<()> {
        unsafe {
            self.logical_device
                .wait_for_fences(&[self.sync_objects.in_flight], true, u64::MAX)?;
            self.logical_device
                .reset_fences(&[self.sync_objects.in_flight])?;
        };

        let (image_index, _) = unsafe {
            self.swapchain.device.acquire_next_image(
                self.swapchain.swapchain,
                u64::MAX,
                self.sync_objects.image_available,
                vk::Fence::null(),
            )?
        };
        unsafe {
            self.logical_device
                .reset_command_buffer(self.command_buffer, vk::CommandBufferResetFlags::empty())?
        };
        self.record_command_buffer(self.command_buffer, image_index)?;

        let submit_info = vk::SubmitInfo {
            s_type: vk::StructureType::SUBMIT_INFO,
            wait_semaphore_count: 1,
            p_wait_semaphores: [self.sync_objects.image_available].as_ptr(),
            p_wait_dst_stage_mask: [vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT].as_ptr(),
            signal_semaphore_count: 1,
            p_signal_semaphores: [self.sync_objects.render_finished].as_ptr(),
            command_buffer_count: 1,
            p_command_buffers: &self.command_buffer,
            ..Default::default()
        };

        unsafe {
            self.logical_device.queue_submit(
                self.queues.graphics,
                &[submit_info],
                self.sync_objects.in_flight,
            )?
        };

        let present_info = vk::PresentInfoKHR {
            s_type: vk::StructureType::PRESENT_INFO_KHR,
            wait_semaphore_count: 1,
            p_wait_semaphores: [self.sync_objects.render_finished].as_ptr(),
            swapchain_count: 1,
            p_swapchains: [self.swapchain.swapchain].as_ptr(),
            p_image_indices: &image_index,
            p_results: null_mut(),
            ..Default::default()
        };

        unsafe {
            self.swapchain
                .device
                .queue_present(self.queues.present, &present_info)?
        };

        Ok(())
    }
}

impl Drop for VulkanRenderer {
    fn drop(&mut self) {
        unsafe {
            _ = self
                .logical_device
                .wait_for_fences(&[self.sync_objects.in_flight], true, u64::MAX);

            self.logical_device
                .destroy_semaphore(self.sync_objects.image_available, None);
            self.logical_device
                .destroy_semaphore(self.sync_objects.render_finished, None);
            self.logical_device
                .destroy_fence(self.sync_objects.in_flight, None);

            self.logical_device
                .destroy_command_pool(self.command_pool, None);

            for framebuffer in self.framebuffers.iter() {
                self.logical_device.destroy_framebuffer(*framebuffer, None);
            }

            self.logical_device
                .destroy_render_pass(self.pipeline.render_pass, None);
            self.logical_device
                .destroy_pipeline(self.pipeline.pipeline, None);
            self.logical_device
                .destroy_pipeline_layout(self.pipeline.layout, None);

            for view in self.swapchain.image_views.iter() {
                self.logical_device.destroy_image_view(*view, None);
            }
            self.swapchain
                .device
                .destroy_swapchain(self.swapchain.swapchain, None);
            self.surface.destroy_surface(self.surface_khr, None);
            self.logical_device.destroy_device(None);

            if let Some(debug) = &self.debug {
                debug
                    .utils
                    .destroy_debug_utils_messenger(debug.messenger, None);
            }

            self.instance.destroy_instance(None);
        }
    }
}

impl std::fmt::Debug for VulkanRenderer {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Vulkan Renderer")?;
        Ok(())
    }
}
