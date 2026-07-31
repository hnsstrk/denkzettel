#include "capture/textareaheight.h"

#include <algorithm>

int capture::textAreaHeight(int documentLines, int lineHeight, int chrome)
{
    return std::clamp(documentLines, MinTextLines, MaxTextLines) * lineHeight + chrome;
}
