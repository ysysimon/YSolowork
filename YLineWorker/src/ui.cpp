#include "ui.h"

#include "ftxui/component/component.hpp"
#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

namespace YLineWorker::UI {
bool button_clicked = false;

Component makeLayout()
{


    // 上方内容
    auto top_content = Renderer([] {
        return vbox({
            text("这是窗口的顶部内容") | center,
        });
    });

    // 底部固定内容
    auto bottom_content = Renderer([] {
        return window (
            text(" 日志 "),
            vbox({
                text("这是固定在窗口底部的内容") | center,
            }) | size(HEIGHT, EQUAL, 10)
        );
    });

    // 使用 filler 将 bottom_content 推到窗口底部
    auto content = Renderer([=] {
        return vbox({
            top_content->Render(),  // 渲染顶部内容
            filler(),               // 占据中间的空间
            bottom_content->Render(), // 渲染固定在底部的内容
        });
    });

   // 再包装最终的 window 
    auto layout = Renderer([=] {
    return window(
        text(" YLine Worker ") | center,  // 窗口的标题
      content->Render()  // 窗口的内容
        );
    });


  return layout;
}

void render(Component layout)
{
    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(layout);
}

} // namespace YLineWorker