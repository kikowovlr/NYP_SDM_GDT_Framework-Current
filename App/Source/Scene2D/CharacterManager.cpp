#include "CharacterManager.h"
#include "Player2D.h"
#include "Topdee.h"

CharacterManager::CharacterManager() : activeCharacter(NULL), toodee(NULL), topdee(NULL), savedToodeeState(NULL), savedTopdeeState(NULL)
{
}

CharacterManager::~CharacterManager()
{
    activeCharacter = NULL;
    topdee = NULL;
    toodee = NULL;

    if (savedToodeeState)
    {
        delete savedToodeeState;
        savedToodeeState = NULL;
    }

    if (savedTopdeeState)
    {
        delete savedTopdeeState;
        savedTopdeeState = NULL;
    }
}

bool CharacterManager::Init(const std::string& shaderName) {
    // Initialize TopDee
    topdee = CTopdee::GetInstance();
    topdee->SetShader(shaderName);
    if (!topdee->Init()) return false;

    // Initialize Toodee
    toodee = CPlayer2D::GetInstance();
    toodee->SetShader(shaderName);
    if (!toodee->Init()) return false;
    
    activeCharacter = toodee; // Default to TopDee
    return true;
}

void CharacterManager::Render()
{
    if (topdee)
    {
        topdee->PreRender();
        topdee->Render();
        topdee->PostRender();
    }
    if (toodee)
    {
        toodee->PreRender();
        toodee->Render();
        toodee->PostRender();
    }
}

void CharacterManager::SwitchCharacter()
{
    // 1. Save current character's state
    if (activeCharacter == topdee) {
        delete savedTopdeeState;  // Delete old state if exists
        savedTopdeeState = new TopdeeState(topdee->SaveState());
    }
    else {  
        delete savedToodeeState;
        savedToodeeState = new ToodeeState(toodee->SaveState());
    }

    // 2. Switch active character
    activeCharacter = (activeCharacter == topdee) ? static_cast<CEntity2D*>(toodee) : static_cast<CEntity2D*>(topdee);

    // 3. Restore the new character's state 
    if (activeCharacter == topdee && savedTopdeeState) {
        topdee->LoadState(*savedTopdeeState);
    }
    else if (activeCharacter == toodee && savedToodeeState) {
        toodee->LoadState(*savedToodeeState);
    }
}

CEntity2D* CharacterManager::GetActiveCharacter()
{
    return activeCharacter;
}

void CharacterManager::UpdateCurrentCharacter(float deltaTime)
{
    if (CEntity2D* current = GetActiveCharacter()) {
        current->Update(deltaTime);  // Calls CTopdee::Update or CToodee::Update
    }
}

void CharacterManager::ResetAllCharacters() {
    topdee->Reset();
    toodee->Reset();
    activeCharacter = toodee;  // Optional: revert to default character
}

void CharacterManager::DeactivateAllCharacters() {
    topdee->SetStatus(false);
    toodee->SetStatus(false);
}

bool CharacterManager::ShouldEndGame()
{
    if (topdee->IsAtExit() && toodee->IsAtExit())
        return true;
}

CEntity2D* CharacterManager::GetTopdee()
{
    return topdee;
}

CEntity2D* CharacterManager::GetToodee()
{
    return toodee;
}
