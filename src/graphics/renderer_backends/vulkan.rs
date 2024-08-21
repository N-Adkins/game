use anyhow::Result;
use ash::{vk, Entry, Instance};

use crate::graphics::Renderer;

/// This literally only exists to implement Debug on ash::Entry.
struct EntryWrapper {
    entry: Entry,
}

impl std::fmt::Debug for EntryWrapper {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "")
    }
}

#[derive(Debug)]
pub struct VulkanRenderer {
    entry: EntryWrapper, // Basically the vulkan context.
}

impl VulkanRenderer {
    pub fn new() -> Result<Self> {
        let entry = unsafe { Entry::load()? };
        let this = Self {
            entry: EntryWrapper { entry },
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
