#pragma once

namespace kernel {
    class Window;
    class Drawable {
        public:
            virtual void Draw (Window& window) const = 0;
    };
}
