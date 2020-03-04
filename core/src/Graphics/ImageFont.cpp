﻿#include "ImageFont.h"
#include <string>
#include "../IO/File.h"
#include "../Logger/Log.h"
#include "Graphics.h"

namespace Altseed {
ImageFont::ImageFont(std::shared_ptr<Font> baseFont) : baseFont_(baseFont), Font() {}

ImageFont::~ImageFont() {}

Vector2I ImageFont::CalcTextureSize(const char16_t* text, WritingDirection direction, bool isEnableKerning) { return Vector2I(); }

void ImageFont::AddImageGlyph(const int32_t character, std::shared_ptr<Texture2D> texture) { imageGlyphs_[character] = texture; }
std::shared_ptr<Texture2D> ImageFont::GetImageGlyph(const int32_t character) {
    if (imageGlyphs_.count(character) > 0) return imageGlyphs_[character];
    return nullptr;
}

std::shared_ptr<ImageFont> ImageFont::CreateImageFont(std::shared_ptr<Font> baseFont) {
    if (baseFont == nullptr) return nullptr;
    return MakeAsdShared<ImageFont>(baseFont);
}
}  // namespace Altseed