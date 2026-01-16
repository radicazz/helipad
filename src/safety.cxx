#include "safety.hxx"

#include <SDL3/SDL.h>
#include <string>

namespace engine {
    bool message_box_info(std::string_view title, std::string_view message) {
        const std::string title_text{title};
        const std::string message_text{message};
        return SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title_text.c_str(),
                                        message_text.c_str(), nullptr);
    }

    bool message_box_error(std::string_view title, std::string_view message) {
        const std::string title_text{title};
        const std::string message_text{message};
        return SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title_text.c_str(),
                                        message_text.c_str(), nullptr);
    }
}  // namespace engine
