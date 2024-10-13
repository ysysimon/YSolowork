#include "UTconsoleUTF8.h"
#include <cstdlib>
#include "ui.h"
 
int main() {
  // set console to UTF-8
  YSolowork::util::setConsoleUTF8();

  // render the UI
  auto layout = YLineWorker::UI::makeLayout();
  YLineWorker::UI::render(layout);

  return EXIT_SUCCESS;
}