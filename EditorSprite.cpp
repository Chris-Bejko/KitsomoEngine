#include "EditorSprite.h"
#include "Engine.h"
#include "Components/Transform.h"
#include "imguiHandler.h"
#include "GizmoSystem.h"
EditorSprite::EditorSprite(const std::string &texId) : textureId(texId) {}

bool EditorSprite::Init()
{
    AssetManager::get().loadTexture(textureId, textureId + ".png");
    texture = AssetManager::get().getTexture(textureId);
    sprite.setTexture(texture);
    sprite.setOrigin((sf::Vector2f)texture.getSize() / 2.f);
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
    sprite.setScale(
        entity->transform->scale.x * 0.05f,
        entity->transform->scale.y * 0.05f);
    auto worldPos = entity->transform->GetWorldPosition();
    sprite.setPosition(worldPos.x, worldPos.y);
    sprite.setRotation(entity->transform->GetWorldRotation());

    // Skip if gizmo is active or collider in edit mode
    // if (GizmoSystem::get().IsGizmoDragging())
    //     return;

    if (entity->HasComponentOfType<Collider>() &&
        entity->GetComponentOfType<Collider>()->editMode)
        return;

    sf::Sprite reference;
    if (entity->HasComponent<Sprite>())
        reference = entity->GetComponent<Sprite>().GetSprite();
    else
        reference = sprite; // fallback to editor icon

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        auto mousePos = Engine::get().GetWindow().mapPixelToCoords(
            sf::Mouse::getPosition(Engine::get().GetWindow()));

        if (IsMouseOver(reference, mousePos) &&
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
                mouseRectOffset = sf::Vector2f(
                    mousePos.x - worldPos.x,
                    mousePos.y - worldPos.y);
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
        auto mousePos = Engine::get().GetWindow().mapPixelToCoords(
            sf::Mouse::getPosition(Engine::get().GetWindow()));

        sf::Vector2f worldTarget(
            mousePos.x - mouseRectOffset.x,
            mousePos.y - mouseRectOffset.y);

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