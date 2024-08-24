use std::ffi::{CStr, CString};

use anyhow::{anyhow, Context, Result};
use ash::{vk, Entry, Instance};
use tracing::{error, instrument, trace};

use crate::graphics::{Renderer, Window};

const VALIDATION_LAYERS: &[&'static str] = &["VK_LAYER_KHRONOS_validation"];

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
pub struct VulkanRenderer {
    entry: EntryWrapper, // Basically the vulkan context.
    instance: InstanceWrapper,
}

impl VulkanRenderer {
    pub fn new(window: &dyn Window) -> Result<Self> {
        let entry = EntryWrapper(unsafe { Entry::load()? });
        let instance = InstanceWrapper(Self::create_instance(&entry, window)?);
        Self::init_validation_layers(&entry)?;
        let this = Self { entry, instance };
        Ok(this)
    }

    fn create_instance(entry: &EntryWrapper, window: &dyn Window) -> Result<Instance> {
        trace!("Attempting to create Vulkan instance");
        let EntryWrapper(entry) = entry;

        let app_info = vk::ApplicationInfo {
            api_version: vk::make_api_version(0, 1, 0, 0),
            ..Default::default()
        };

        let extensions = window.get_requested_extensions();

        // We have to have a separate vec owning the CStrings or else we get dumb stuff happening
        // when using the Vulkan API
        let extension_cstrs: Vec<CString> = extensions
            .iter()
            .map(|s| CString::new(s.clone()).with_context(|| "Failed to parse CString"))
            .collect::<Result<Vec<CString>>>()?;

        let vulkan_extensions: Vec<*const i8> =
            extension_cstrs.iter().map(|s| s.as_ptr()).collect();

        let create_info = vk::InstanceCreateInfo {
            p_application_info: &app_info,
            enabled_extension_count: vulkan_extensions.len() as u32,
            pp_enabled_extension_names: vulkan_extensions.as_ptr(),
            ..Default::default()
        };

        let instance = unsafe { entry.create_instance(&create_info, None)? };

        trace!("Created Vulkan instance");

        Ok(instance)
    }

    fn init_validation_layers(entry: &EntryWrapper) -> Result<()> {
        trace!("Attempting to initialize Vulkan validation layers");
        let EntryWrapper(entry) = entry;

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
                        .unwrap()
                        .to_str()
                        .unwrap()
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

        trace!("Initialized Vulkan validation layers");

        Ok(())
    }
}
