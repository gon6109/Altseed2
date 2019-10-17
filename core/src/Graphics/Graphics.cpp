#include "Graphics.h"

#include "Sprite.h"

#ifdef _WIN32
#pragma comment(lib, "d3dcompiler.lib")

#ifdef ENABLE_VULKAN
#pragma comment(lib, "vulkan-1.lib")
#endif

#endif

namespace altseed {

std::shared_ptr<Graphics> Graphics::instance = nullptr;

std::shared_ptr<Graphics>& Graphics::GetInstance() { return instance; }

bool Graphics::Initialize(std::shared_ptr<Window>& window, LLGI::DeviceType deviceType) {
    instance = std::make_shared<Graphics>();

    instance->window_ = window;
    instance->llgiWindow_ = std::make_shared<LLGIWindow>(window->GetNativeWindow());
    instance->platform_ = LLGI::CreatePlatform(deviceType, instance->llgiWindow_.get());
    if (instance->platform_ == nullptr) return false;

    instance->graphics_ = instance->platform_->CreateGraphics();
    // ? instance->graphics_->SetDisposed([]() -> void { instance->platform_->Release(); });
    if (instance->graphics_ == nullptr) return false;

    instance->sfMemoryPool_ = instance->graphics_->CreateSingleFrameMemoryPool(1024 * 1024, 128);
    for (int i = 0; i < instance->commandLists_.size(); i++)
        instance->commandLists_[i] = instance->graphics_->CreateCommandList(instance->sfMemoryPool_);

    instance->vb = instance->graphics_->CreateVertexBuffer(sizeof(SimpleVertex) * 1024);
    instance->ib = instance->graphics_->CreateIndexBuffer(2, 1024);

    instance->compiler_ = LLGI::CreateCompiler(deviceType);

    instance->vs_ = instance->CreateShader(instance->HlslVSCode, LLGI::ShaderStageType::Vertex);

    return true;
}

bool Graphics::Update() {
    if (!platform_->NewFrame()) return false;

    instance->sfMemoryPool_->NewFrame();

    count++;

    LLGI::Color8 color;
    color.R = count % 255;
    color.G = 0;
    color.B = 0;
    color.A = 255;

    UpdateBuffers();

    auto renderPass = instance->platform_->GetCurrentScreen(color, true);
    auto renderPassPipelineState = LLGI::CreateSharedPtr(instance->graphics_->CreateRenderPassPipelineState(renderPass));
    if (pips.count(renderPassPipelineState) == 0) {
        auto pip = graphics_->CreatePiplineState();
        pip->VertexLayouts[0] = LLGI::VertexLayoutFormat::R32G32B32_FLOAT;
        pip->VertexLayouts[1] = LLGI::VertexLayoutFormat::R32G32_FLOAT;
        pip->VertexLayouts[2] = LLGI::VertexLayoutFormat::R8G8B8A8_UNORM;
        pip->VertexLayoutNames[0] = "POSITION";
        pip->VertexLayoutNames[1] = "UV";
        pip->VertexLayoutNames[2] = "COLOR";
        pip->VertexLayoutCount = 3;

        pip->Culling = LLGI::CullingMode::DoubleSide;
        pip->SetShader(LLGI::ShaderStageType::Vertex, vs_->Get());
        pip->SetShader(LLGI::ShaderStageType::Pixel, Sprites[0]->GetMaterial()->GetShader()->Get());
        pip->SetRenderPassPipelineState(renderPassPipelineState.get());
        pip->Compile();

        pips[renderPassPipelineState] = LLGI::CreateSharedPtr(pip);
    }
    auto commandList = commandLists_[count % commandLists_.size()];
    commandList->Begin();
    commandList->BeginRenderPass(renderPass);

    for (auto& g : Groups) {
        commandList->SetVertexBuffer(vb, sizeof(SimpleVertex), g.second.vb_offset * sizeof(SimpleVertex));
        commandList->SetIndexBuffer(ib);
        commandList->SetPipelineState(pips[renderPassPipelineState].get());
        commandList->SetTexture(
                g.first.get(), LLGI::TextureWrapMode::Repeat, LLGI::TextureMinMagFilter::Nearest, 0, LLGI::ShaderStageType::Pixel);
        commandList->Draw(g.second.sprites.size() * 2);
    }
    commandList->EndRenderPass();
    commandList->End();
    graphics_->Execute(commandList);

    platform_->Present();

    count++;

    return true;
}

void Graphics::Terminate() {
    instance->graphics_->WaitFinish();
    LLGI::SafeRelease(instance->sfMemoryPool_);
    for (int i = 0; i < instance->commandLists_.size(); i++) LLGI::SafeRelease(instance->commandLists_[i]);
    LLGI::SafeRelease(instance->ib);
    LLGI::SafeRelease(instance->vb);
    LLGI::SafeRelease(instance->graphics_);
    LLGI::SafeRelease(instance->platform_);
    instance = nullptr;
}

void Graphics::UpdateBuffers() {
    std::array<LLGI::Vec2F, 4> UVs = {LLGI::Vec2F(0.0f, 0.0f), LLGI::Vec2F(1.0f, 0.0f), LLGI::Vec2F(1.0f, 1.0f), LLGI::Vec2F(0.0f, 1.0f)};
    auto vb_buf = reinterpret_cast<SimpleVertex*>(vb->Lock());
    auto ib_buf = reinterpret_cast<uint16_t*>(ib->Lock());

    int vb_idx = 0;
    int maxPrimitiveCount = 0;

    Groups.clear();
    for (int i = 0; i < Sprites.size(); i++) {
        Groups[Sprites[i]->GetTexture()].sprites.push_back(Sprites[i]);
    }

    for (auto& g : Groups) {
        g.second.vb_offset = vb_idx;

        // TODO: check size of buffers
        for (int i = 0; i < g.second.sprites.size(); i++) {
            auto s = g.second.sprites[i];
            auto v = s->GetVertex(llgiWindow_->GetWindowSize());

            for (int j = 0; j < 4; j++) {
                vb_buf[vb_idx + j].Pos = {v[j].X, v[j].Y, 0.5f};
                vb_buf[vb_idx + j].UV = UVs[j];
                vb_buf[vb_idx + j].Color = LLGI::Color8();
            }

            vb_idx += 4;
        }
        maxPrimitiveCount = std::max<int>(maxPrimitiveCount, g.second.sprites.size());
    }

    for (int i = 0; i < maxPrimitiveCount; i++) {
        ib_buf[i * 6 + 0] = i * 4 + 0;
        ib_buf[i * 6 + 1] = i * 4 + 1;
        ib_buf[i * 6 + 2] = i * 4 + 2;
        ib_buf[i * 6 + 3] = i * 4 + 0;
        ib_buf[i * 6 + 4] = i * 4 + 2;
        ib_buf[i * 6 + 5] = i * 4 + 3;
    }

    ib->Unlock();
    vb->Unlock();
}

std::shared_ptr<Shader> Graphics::CreateShader(const char* code, LLGI::ShaderStageType shaderStageType) {
    if (instance->compiler_ == nullptr) return nullptr;

    std::vector<LLGI::DataStructure> data;
    LLGI::CompilerResult result;

    if (instance->platform_->GetDeviceType() == LLGI::DeviceType::DirectX12) {
        instance->compiler_->Compile(result, code, shaderStageType);
        assert(result.Message == "");
    } else
        return nullptr;

    for (auto& b : result.Binary) {
        LLGI::DataStructure d;
        d.Data = b.data();
        d.Size = b.size();
        data.push_back(d);
    }

    auto obj = std::make_shared<Shader>();
    obj->SetShaderBinary(instance->graphics_->CreateShader(data.data(), data.size()));

    return obj;
}

std::shared_ptr<LLGI::Texture> Graphics::CreateDameyTexture(uint8_t b) {
    std::shared_ptr<LLGI::Texture> texture(graphics_->CreateTexture(LLGI::Vec2I(256, 256), false, false));
    auto texture_buf = (LLGI::Color8*)texture->Lock();
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            texture_buf[x + y * 256].R = x;
            texture_buf[x + y * 256].G = y;
            texture_buf[x + y * 256].B = b;
            texture_buf[x + y * 256].A = 255;
        }
    }
    texture->Unlock();
    return texture;
}

std::shared_ptr<LLGI::Texture> Graphics::CreateTexture(uint8_t* data, int32_t width, int32_t height, int32_t channel) {
    std::shared_ptr<LLGI::Texture> texture(graphics_->CreateTexture(LLGI::Vec2I(width, height), true, false));
    if (texture == nullptr) return nullptr;

    auto texture_buf = (LLGI::Color8*)texture->Lock();
    if (channel == 4) {
        for (int32_t y = 0; y < height; y++) {
            for (int32_t x = 0; x < width; x++) {
                texture_buf[x + y * width].R = data[4 * (x + y * width)];
                texture_buf[x + y * width].G = data[4 * (x + y * width) + 1];
                texture_buf[x + y * width].B = data[4 * (x + y * width) + 2];
                texture_buf[x + y * width].A = data[4 * (x + y * width) + 3];
            }
        }
    } else if (channel == 3) {
        for (int32_t y = 0; y < height; y++) {
            for (int32_t x = 0; x < width; x++) {
                texture_buf[x + y * width].R = data[3 * (x + y * width)];
                texture_buf[x + y * width].G = data[3 * (x + y * width) + 1];
                texture_buf[x + y * width].B = data[3 * (x + y * width) + 2];
                texture_buf[x + y * width].A = 255;
            }
        }
    }
    texture->Unlock();
    return texture;
}

}  // namespace altseed
