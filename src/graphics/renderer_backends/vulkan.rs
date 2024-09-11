use std::{
    collections::BTreeMap,
    ffi::{CStr, CString},
    ptr::null,
    sync::OnceLock,
};

use anyhow::{anyhow, Context, Result};
use ash::{vk, Entry, Instance};
use tracing::{error, info, trace, warn};

use crate::graphics::Window;

const VALIDATION_LAYERS: &[&str] = &["VK_LAYER_KHRONOS_validation"];

static ENABLE_VALIDATION_LAYERS: OnceLock<bool> = OnceLock::new();

struct EntryWrapper(Entry);

impl std::fmt::Debug for EntryWrapper {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Vulkan Entry")
    }
}

struct InstanceWrapper(Instance);

impl std::fmt::Debug for InstanceWrapper {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Vulkan Instance")
    }
}

#[derive(Debug)]
struct PhysicalDeviceWrapper(vk::PhysicalDevice);

#[allow(unused)]
struct LogicalDeviceWrapper(ash::Device);

impl std::fmt::Debug for LogicalDeviceWrapper {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Vulkan Logical Device")
    }
}

struct VulkanDebug {
    utils: ash::ext::debug_utils::Instance,
    messenger: vk::DebugUtilsMessengerEXT,
}

impl std::fmt::Debug for VulkanDebug {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Vulkan Debug Data")
    }
}

#[derive(Debug, Default)]
struct QueueFamilies {
    graphics_family: Option<u32>,
}

impl QueueFamilies {
    fn is_complete(&self) -> bool {
        self.graphics_family.is_some()
    }
}

#[allow(unused)]
#[derive(Debug)]
pub struct VulkanRenderer {
    entry: EntryWrapper, // Basically the vulkan context.
    instance: InstanceWrapper,
    debug: Option<VulkanDebug>,
    physical_device: PhysicalDeviceWrapper,
    queue_families: QueueFamilies,
    logical_device: LogicalDeviceWrapper,
}

impl VulkanRenderer {
    pub fn new(window: &dyn Window) -> Result<Self> {
        let enable_validation_layers: bool = match std::env::var("ENABLE_VALIDATION_LAYERS") {
            Ok(s) => s.parse().unwrap_or(false),
            Err(_) => false,
        };
        _ = ENABLE_VALIDATION_LAYERS.set(enable_validation_layers);

        let entry = EntryWrapper(unsafe { Entry::load()? });
        let instance = InstanceWrapper(Self::create_instance(&entry, window)?);
        let debug = if *ENABLE_VALIDATION_LAYERS.get().unwrap() {
            Self::check_validation_layers(&entry)?;
            Some(Self::create_debug(&entry, &instance)?)
        } else {
            None
        };
        let physical_device = PhysicalDeviceWrapper(Self::pick_physical_device(&instance)?);
        let queue_families = Self::get_queue_families(&instance, &physical_device);
        let logical_device = LogicalDeviceWrapper(Self::create_logical_device(
            &instance,
            &physical_device,
            &queue_families,
        )?);

        let this = Self {
            entry,
            instance,
            debug,
            physical_device,
            queue_families,
            logical_device,
        };

        Ok(this)
    }

    fn create_instance(entry_wrapper: &EntryWrapper, window: &dyn Window) -> Result<Instance> {
        trace!("Attempting to create Vulkan instance");
        let EntryWrapper(entry) = entry_wrapper;

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

    fn check_validation_layers(entry_wrapper: &EntryWrapper) -> Result<()> {
        trace!("Checking supported Vulkan validation layers");
        let EntryWrapper(entry) = entry_wrapper;

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

    fn create_debug(
        entry_wrapper: &EntryWrapper,
        instance_wrapper: &InstanceWrapper,
    ) -> Result<VulkanDebug> {
        trace!("Attempting to create Vulkan Debug Messenger");
        let EntryWrapper(entry) = entry_wrapper;
        let InstanceWrapper(instance) = instance_wrapper;
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

    fn rank_device(
        instance_wrapper: &InstanceWrapper,
        device_wrapper: &PhysicalDeviceWrapper,
    ) -> usize {
        let InstanceWrapper(instance) = instance_wrapper;
        let PhysicalDeviceWrapper(device) = device_wrapper;
        let properties = unsafe { instance.get_physical_device_properties(*device) };
        let features = unsafe { instance.get_physical_device_features(*device) };

        let families = Self::get_queue_families(instance_wrapper, device_wrapper);

        if features.geometry_shader == 0 || !families.is_complete() {
            return 0; // Necessary for usage
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

        score
    }

    fn pick_physical_device(instance_wrapper: &InstanceWrapper) -> Result<vk::PhysicalDevice> {
        trace!("Attempting to pick Vulkan Physical Device");

        let InstanceWrapper(instance) = instance_wrapper;
        let devices = unsafe { instance.enumerate_physical_devices()? };
        let mut candidates: BTreeMap<usize, vk::PhysicalDevice> = BTreeMap::new();

        for device in devices {
            let score = Self::rank_device(instance_wrapper, &PhysicalDeviceWrapper(device));
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
        instance_wrapper: &InstanceWrapper,
        device_wrapper: &PhysicalDeviceWrapper,
    ) -> QueueFamilies {
        trace!("Getting queue families");

        let InstanceWrapper(instance) = instance_wrapper;
        let PhysicalDeviceWrapper(device) = device_wrapper;

        let mut families = QueueFamilies {
            ..Default::default()
        };

        let properties = unsafe { instance.get_physical_device_queue_family_properties(*device) };
        for (i, property) in properties.into_iter().enumerate() {
            #[allow(clippy::single_match)]
            if property.queue_flags.contains(vk::QueueFlags::GRAPHICS) {
                families.graphics_family = Some(i as u32);
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
        instance_wrapper: &InstanceWrapper,
        physical_device_wrapper: &PhysicalDeviceWrapper,
        queue_families: &QueueFamilies,
    ) -> Result<ash::Device> {
        trace!("Attempting to create Vulkan logical device");

        let InstanceWrapper(instance) = instance_wrapper;
        let PhysicalDeviceWrapper(physical_device) = physical_device_wrapper;

        let queue_priorities: f32 = 1.0;
        let queue_info = vk::DeviceQueueCreateInfo {
            s_type: vk::StructureType::DEVICE_QUEUE_CREATE_INFO,
            queue_family_index: queue_families.graphics_family.unwrap(),
            queue_count: 1,
            p_queue_priorities: &queue_priorities,
            ..Default::default()
        };

        let device_features = vk::PhysicalDeviceFeatures {
            ..Default::default()
        };
        let device_info = vk::DeviceCreateInfo {
            s_type: vk::StructureType::DEVICE_CREATE_INFO,
            p_queue_create_infos: &queue_info,
            queue_create_info_count: 1,
            p_enabled_features: &device_features,
            ..Default::default()
        };
        let device = unsafe { instance.create_device(*physical_device, &device_info, None)? };

        trace!("Created Vulkan logical device");

        Ok(device)
    }
}

impl Drop for VulkanRenderer {
    fn drop(&mut self) {
        unsafe {
            let LogicalDeviceWrapper(logical_device) = &self.logical_device;
            let InstanceWrapper(instance) = &self.instance;

            logical_device.destroy_device(None);

            if let Some(debug) = &self.debug {
                debug
                    .utils
                    .destroy_debug_utils_messenger(debug.messenger, None);
            }
            instance.destroy_instance(None);
        }
    }
}
