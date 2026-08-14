#include "EditorSprite.h"
#include "Engine.h"
#include "Transform.h"
#include "imguiHandler.h"
#include "GizmoSystem.h"
#include "UIRect.h"
#include "Canvas.h"
#include "ComponentRegistry.h"

REGISTER_COMPONENT(EditorSprite)
bool EditorSprite::Init()
{
    std::string textureId = "Assets/Textures/editor_entity.png"; // default editor icon
    _texture.SetPath(textureId);
    AssetManager::get().loadTexture(_texture.GetPath());
    texture = AssetManager::get().getTexture(_texture.GetPath());
    // AssetManager::get().loadTexture(textureId, textureId + ".png");
    // texture = AssetManager::get().getTexture(textureId);
    sprite.setTexture(*texture);
    sprite.setOrigin((sf::Vector2f)texture->getSize() / 2.f);
    sprite.setScale(0.05f, 0.05f);
    return true;
}

void EditorSprite::draw()
{
    if (!Engine::get().isEngine)
        return; // invisible in play mode
    Engine::get().GetWindow().draw(sprite);
}
void EditorSprite::updateEngine(float dt)
{
    bool isUI = entity->HasComponent<UIRect>();

    // Get correct mouse positions
    sf::Vector2i pixelPos = sf::Mouse::getPosition(Engine::get().GetWindow());
    sf::View defaultView = Engine::get().GetWindow().getDefaultView();
    
    // Screen space mouse (for UI) - use default view mapping
    sf::Vector2f mouseScreen = Engine::get().GetWindow().mapPixelToCoords(pixelPos, defaultView);
    // World space mouse (for world entities)
    sf::Vector2f mouseWorld = Engine::get().GetWindow().mapPixelToCoords(pixelPos);

    // Update sprite position
    if (isUI)
    {
        auto& ui = entity->GetComponent<UIRect>();
        auto rect = ui.GetScreenRect();
        sf::Vector2f center(rect.left + rect.width * 0.5f,
                            rect.top  + rect.height * 0.5f);
        // Convert screen pos to world pos for sprite rendering
        auto worldPos = Engine::get().GetWindow().mapPixelToCoords(
            sf::Vector2i((int)center.x, (int)center.y), defaultView);
        sprite.setPosition(worldPos);
    }
    else
    {
        auto worldPos = entity->transform->GetWorldPosition();
        sprite.setPosition(worldPos.x, worldPos.y);
    }

    sprite.setScale(entity->transform->scale.x * 0.01f,
                    entity->transform->scale.y * 0.01f);
    sprite.setRotation(entity->transform->GetWorldRotation());

    if (entity->HasComponentOfType<Collider>() &&
        entity->GetComponentOfType<Collider>()->editMode)
        return;

    // Hit detection
    bool mouseOver = false;
    if (isUI)
    {
        auto& ui = entity->GetComponent<UIRect>();
        mouseOver = ui.GetScreenRect().contains(mouseScreen);
    }
    else if (entity->HasComponent<Sprite>())
        mouseOver = entity->GetComponent<Sprite>().GetGlobalBounds().contains(mouseWorld);
    else
        mouseOver = sprite.getGlobalBounds().contains(mouseWorld);

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        if (mouseOver &&
            (!Engine::get().DraggingEntity() ||
             Engine::get().GetDraggedEntity() == entity->GetName()))
        {
            if (!dragging && !pendingDrag)
            {
                GizmoSystem::get().SetSelectedEntity(entity);
                float currentTime = ImGui::GetTime();
                if (lastClickTime > 0.f && (currentTime - lastClickTime) < 0.3f)
                {
                    Engine::get().FocusOnEntity(entity);
                    lastClickTime = 0.f;
                }
                else
                    lastClickTime = currentTime;

                pendingDrag = true;
                dragTimer = 0.f;

                if (isUI)
                {
                    auto& ui = entity->GetComponent<UIRect>();
                    dragStartAnchorOffset = {ui.anchorOffset.x, ui.anchorOffset.y};
                    dragStartMousePos = mouseScreen; // now correctly mapped
                }
                else
                {
                    mouseRectOffset = mouseWorld - sf::Vector2f(
                        entity->transform->GetWorldPosition().x,
                        entity->transform->GetWorldPosition().y);
                }
            }

            if (pendingDrag)
            {
                dragTimer += dt;
                if (dragTimer >= dragDelay)
                {
                    Engine::get().TriggerDragging(entity->GetName());
                    ImguiHandler::get().ClearInspector();
                    entity->displayComponents = true;
                    Engine::get().GetManager()->SetSelectedEntity(entity);
                    dragging = true;
                    pendingDrag = false;
                }
            }
        }
    }
    else
    {
        Engine::get().TriggerDragging("");
        dragging = false;
        pendingDrag = false;
        dragTimer = 0.f;
    }

    if (dragging)
    {
        if (entity->HasComponent<Canvas>()) return;

        if (isUI)
        {
            auto& ui = entity->GetComponent<UIRect>();
            sf::Vector2f delta = mouseScreen - dragStartMousePos;
            ui.anchorOffset.x = dragStartAnchorOffset.x + delta.x;
            ui.anchorOffset.y = dragStartAnchorOffset.y + delta.y;
        }
        else
        {
            sf::Vector2f worldTarget = mouseWorld - mouseRectOffset;

            if (entity->transform->GetParent() != nullptr)
            {
                Vector2F parentWorld = entity->transform->GetParent()->GetWorldPosition();
                float parentRot = entity->transform->GetParent()->GetWorldRotation() * 3.14159f / 180.f;
                Vector2F parentScale = entity->transform->GetParent()->GetWorldScale();
                float dx = worldTarget.x - parentWorld.x;
                float dy = worldTarget.y - parentWorld.y;
                float cosR = cos(-parentRot);
                float sinR = sin(-parentRot);
                entity->transform->position = Vector2F(
                    (dx * cosR - dy * sinR) / parentScale.x,
                    (dx * sinR + dy * cosR) / parentScale.y);
            }
            else
            {
                entity->transform->position = Vector2F(worldTarget.x, worldTarget.y);
            }
        }
    }
}