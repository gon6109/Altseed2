#include "Graphics/Graphics.h"

#include <Core.h>
#include <gtest/gtest.h>

#include <memory>

#include "Graphics/Sprite.h"

namespace asd = altseed;

TEST(Grapgics, Initialize) {
    EXPECT_TRUE(asd::Core::Initialize(u"test", 1280, 720, asd::CoreOption()));

    int count = 0;

    auto instance = altseed::Graphics::GetInstance();

    auto t1 = instance->CreateDameyTexture(0);
    auto t2 = instance->CreateDameyTexture(255);

    auto shader = instance->CreateShader(instance->HlslPSCode);
    auto material = std::make_shared<altseed::Material>();
    material->SetShader(shader);

    {
        int c = 0;
        for (int x = 0; x <= 3; x++) {
            for (int y = 0; y <= 3; y++) {
                auto sprite = std::make_shared<altseed::Sprite>();
                sprite->SetMaterial(material);
                sprite->SetPosition(asd::Vector2DF(x * 120, y * 120));
                sprite->SetSize(asd::Vector2DF(80, 80));
                sprite->SetTexture(c++ % 2 == 0 ? t1 : t2);
                instance->Sprites.push_back(sprite);
            }
        }
    }

    while (count++ < 1000 && instance->DoEvents()) EXPECT_TRUE(instance->Update());

    altseed::Graphics::Terminate();
}

TEST(Grapgics, Texture) {
    EXPECT_TRUE(asd::Core::Initialize(u"test", 1280, 720, asd::CoreOption()));

    int count = 0;

    auto instance = altseed::Graphics::GetInstance();

    auto t1 = asd::Texture2D::Load(u"TestData/IO/AltseedPink256.png");
    auto t2 = asd::Texture2D::Load(u"TestData/IO/AltseedPink.jpg");

    auto shader = instance->CreateShader(instance->HlslPSCode);
    auto material = std::make_shared<altseed::Material>();
    material->SetShader(shader);

    {
        int c = 0;
        for (int x = 0; x <= 3; x++) {
            for (int y = 0; y <= 3; y++) {
                auto sprite = std::make_shared<altseed::Sprite>();
                sprite->SetMaterial(material);
                sprite->SetPosition(asd::Vector2DF(x * 120, y * 120));
                sprite->SetSize(asd::Vector2DF(80, 80));
                sprite->SetTexture(c++ % 2 == 0 ? t1->GetNativeTexture() : t2->GetNativeTexture());
                instance->Sprites.push_back(sprite);
            }
        }
    }

    while (count++ < 1000 && instance->DoEvents()) EXPECT_TRUE(instance->Update());

    altseed::Graphics::Terminate();
}
