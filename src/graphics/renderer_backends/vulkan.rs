use std::{
    collections::{BTreeMap, HashSet},
    ffi::{CStr, CString},
    ptr::null,
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

#[derive(Debug)]
struct SwapChainSupport {
    capabilities: vk::SurfaceCapabilitiesKHR,
    formats: Vec<vk::SurfaceFormatKHR>,
    present_modes: Vec<vk::PresentModeKHR>,
}

impl SwapChainSupport {
    fn supported(&self) -> bool {
        !self.formats.is_empty() && !self.present_modes.is_empty()
    }
}

#[allow(unused)]
pub struct VulkanRenderer {
    entry: ash::Entry, // Basically the vulkan context.
    instance: ash::Instance,
    debug: Option<VulkanDebug>,
    physical_device: vk::PhysicalDevice,
    queue_families: QueueFamilies,
    logical_device: ash::Device,
    surface: surface::Instance,
    surface_khr: vk::SurfaceKHR,
    swapchain_device: swapchain::Device,
    swapchain: vk::SwapchainKHR,
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
        let logical_device =
            Self::create_logical_device(&instance, &physical_device, &queue_families)?;

        let swapchain_device = Self::create_swapchain_device(&instance, &logical_device);
        let swapchain = Self::create_swapchain(
            &swapchain_device,
            &instance,
            &surface,
            &surface_khr,
            &physical_device,
            window,
        )?;

        let this = Self {
            entry,
            instance,
            debug,
            physical_device,
            queue_families,
            logical_device,
            surface,
            surface_khr,
            swapchain_device,
            swapchain,
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
    ) -> Result<ash::Device> {
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

        trace!("Created Vulkan logical device");

        Ok(device)
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

    fn create_swapchain_device(
        instance: &ash::Instance,
        device: &ash::Device,
    ) -> swapchain::Device {
        trace!("Creating Vulkan swapchain device");
        swapchain::Device::new(instance, device)
    }

    fn create_swapchain(
        swapchain_device: &swapchain::Device,
        instance: &ash::Instance,
        surface: &surface::Instance,
        surface_khr: &vk::SurfaceKHR,
        device: &vk::PhysicalDevice,
        window: &dyn Window,
    ) -> Result<vk::SwapchainKHR> {
        trace!("Attempting to create Vulkan swapchain");

        let support = Self::get_swapchain_support(surface, surface_khr, device)?;
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

        let families = Self::get_queue_families(instance, device, surface, surface_khr);
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

        trace!("Created Vulkan swapchain");

        Ok(swapchain)
    }
}

impl Drop for VulkanRenderer {
    fn drop(&mut self) {
        unsafe {
            self.swapchain_device
                .destroy_swapchain(self.swapchain, None);
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
