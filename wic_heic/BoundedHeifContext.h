#pragma once

// Own the C context so callers can set libheif's public security limits without
// modifying the dependency's C++ wrapper or relying on its private layout.
class BoundedHeifContext
{
    std::unique_ptr<heif_context, decltype(&heif_context_free)> m_Context;
    static void Check(heif_error error) {
        if (error.code != heif_error_Ok) throw heif::Error(error);
    }
public:
    BoundedHeifContext() : m_Context(heif_context_alloc(), heif_context_free) {
        if (!m_Context) throw std::bad_alloc();
    }
    heif_context* native_context() noexcept { return m_Context.get(); }
    void read_from_reader(heif::Context::Reader& reader) {
        static const heif_reader callbacks = {
            1,
            [](void* user) -> int64_t { return static_cast<heif::Context::Reader*>(user)->get_position(); },
            [](void* data, size_t size, void* user) -> int { return static_cast<heif::Context::Reader*>(user)->read(data, size); },
            [](int64_t position, void* user) -> int { return static_cast<heif::Context::Reader*>(user)->seek(position); },
            [](int64_t target, void* user) -> heif_reader_grow_status { return static_cast<heif::Context::Reader*>(user)->wait_for_file_size(target); }
        };
        Check(heif_context_read_from_reader(m_Context.get(), &callbacks, &reader, nullptr));
    }
    int get_number_of_top_level_images() const noexcept {
        return heif_context_get_number_of_top_level_images(m_Context.get());
    }
    std::vector<heif_item_id> get_list_of_top_level_image_IDs() const {
        std::vector<heif_item_id> ids(get_number_of_top_level_images());
        heif_context_get_list_of_top_level_image_IDs(m_Context.get(), ids.data(), static_cast<int>(ids.size()));
        return ids;
    }
    heif::ImageHandle get_primary_image_handle() const {
        heif_image_handle* image = nullptr;
        Check(heif_context_get_primary_image_handle(m_Context.get(), &image));
        return heif::ImageHandle(image);
    }
    heif::ImageHandle get_image_handle(heif_item_id id) const {
        heif_image_handle* image = nullptr;
        Check(heif_context_get_image_handle(m_Context.get(), id, &image));
        return heif::ImageHandle(image);
    }
};
