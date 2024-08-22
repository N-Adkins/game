use anyhow::Result;
use ash::{vk, Entry, Instance};

use crate::graphics::Renderer;

/// This literally only exists to implement Debug on ash::Entry.
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
    pub fn new() -> Result<Self> {
        let entry = unsafe { Entry::load()? };
        let instance = Self::create_instance(&entry)?;
        let this = Self {
            entry: EntryWrapper(entry),
            instance: InstanceWrapper(instance),
        };
        Ok(this)
    }

    fn create_instance(entry: &Entry) -> Result<Instance> {
        let app_info = vk::ApplicationInfo {
            api_version: vk::make_api_version(0, 1, 0, 0),
            ..Default::default()
        };

        let create_info = vk::InstanceCreateInfo {
            p_application_info: &app_info,
            ..Default::default()
        };

        let instance = unsafe { entry.create_instance(&create_info, None)? };

        Ok(instance)
    }
}
