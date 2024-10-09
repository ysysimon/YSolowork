#ifndef YLINEWORKER_UI_H
#define YLINEWORKER_UI_H

#include "ftxui/component/component_base.hpp"

namespace YLineWorker::UI {
using namespace ftxui;

Component makeLayout();

void render(Component layout);

} // namespace YLineWorker


#endif // YLINEWORKER_UI_H