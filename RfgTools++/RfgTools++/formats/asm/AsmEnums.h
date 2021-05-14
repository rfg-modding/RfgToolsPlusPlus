#pragma once
#include "common/Typedefs.h"
#include <stdexcept>

enum class AllocatorType : u8
{
    None = 0,
    World = 1,
    ChunkPreload = 2,
    EffectPreload = 3,
    EffectCutscene = 4,
    ItemPreload = 5,
    DecalPreload = 6,
    ClothSimPreload = 7,
    Tod = 8,
    MpEffectPreload = 9,
    MpItemPreload = 10,
    Player = 11,
    Human = 12,
    LargeWeapon = 13,
    SmallWeapon = 14,
    Vehicle = 15,
    LargeLayer = 16,
    SmallLayer = 17,
    HumanVoicePersona = 18,
    AlwaysLoadedHumanVoicePersona = 19,
    Audio = 20,
    Interface = 21,
    Fsm = 22,
    InterfaceStack = 23,
    InterfaceSlot = 24,
    InterfaceMpPreload = 25,
    InterfaceMpSlot = 26,
    MaterialEffect = 27,
    Permanent = 28,
    DlcEffectPreload = 29,
    DlcItemPreload = 30,
    NumAllocatorTypes = 31,
};

enum class ContainerType : u8
{
    None = 0,
    Glass = 1,
    EffectsEnv = 2,
    EffectsPreload = 3,
    EffectsDlc = 4,
    MpEffects = 5,
    LayerSmall = 6,
    LayerLarge = 7,
    Audio = 8,
    ClothSim = 9,
    Decals = 10,
    DecalsPreload = 11,
    Fsm = 12,
    Ui = 13,
    Env = 14,
    Chunk = 15,
    ChunkPreload = 16,
    Stitch = 17,
    World = 18,
    HumanHead = 19,
    Human = 20,
    Player = 21,
    Items = 22,
    ItemsPreload = 23,
    ItemsMpPreload = 24,
    ItemsDlc = 25,
    WeaponLarge = 26,
    WeaponSmall = 27,
    Skybox = 28,
    Vehicle = 29,
    VoicePersona = 30,
    AlwaysLoadedVoicePersona = 31,
    Foliage = 32,
    UiPeg = 33,
    MaterialEffect = 34,
    MaterialPreload = 35,
    SharedBackpack = 36,
    LandmarkLod = 37,
    GpsPreload = 38,
    NumContainerTypes = 39
};

enum class PrimitiveType : u8
{
    None = 0,
    Peg = 1,
    Chunk = 2,
    Zone = 3,
    Terrain = 4,
    StaticMesh = 5,
    CharacterMesh = 6,
    FoliageMesh = 7,
    Material = 8,
    ClothSim = 9,
    Vehicle = 10,
    VehicleAudio = 11,
    Vfx = 12,
    Wavebank = 13,
    FoleyBank = 14,
    MeshMorph = 15,
    VoicePersona = 16,
    AnimFile = 17,
    Vdoc = 18,
    LuaScript = 19,
    Localization = 20,
    TerrainHighLod = 21,
    LandmarkLod = 22,
    NumPrimitiveTypes = 23,
};

static string to_string(AllocatorType value)
{
    switch (value)
    {
    case AllocatorType::None:
        return "None";
    case AllocatorType::World:
        return "World";
    case AllocatorType::ChunkPreload:
        return "ChunkPreload";
    case AllocatorType::EffectPreload:
        return "EffectPreload";
    case AllocatorType::EffectCutscene:
        return "EffectCutscene";
    case AllocatorType::ItemPreload:
        return "ItemPreload";
    case AllocatorType::DecalPreload:
        return "DecalPreload";
    case AllocatorType::ClothSimPreload:
        return "ClothSimPreload";
    case AllocatorType::Tod:
        return "Tod";
    case AllocatorType::MpEffectPreload:
        return "MpEffectPreload";
    case AllocatorType::MpItemPreload:
        return "MpItemPreload";
    case AllocatorType::Player:
        return "Player";
    case AllocatorType::Human:
        return "Human";
    case AllocatorType::LargeWeapon:
        return "LargeWeapon";
    case AllocatorType::SmallWeapon:
        return "SmallWeapon";
    case AllocatorType::Vehicle:
        return "Vehicle";
    case AllocatorType::LargeLayer:
        return "LargeLayer";
    case AllocatorType::SmallLayer:
        return "SmallLayer";
    case AllocatorType::HumanVoicePersona:
        return "HumanVoicePersona";
    case AllocatorType::AlwaysLoadedHumanVoicePersona:
        return "AlwaysLoadedHumanVoicePersona";
    case AllocatorType::Audio:
        return "Audio";
    case AllocatorType::Interface:
        return "Interface";
    case AllocatorType::Fsm:
        return "Fsm";
    case AllocatorType::InterfaceStack:
        return "InterfaceStack";
    case AllocatorType::InterfaceSlot:
        return "InterfaceSlot";
    case AllocatorType::InterfaceMpPreload:
        return "InterfaceMpPreload";
    case AllocatorType::InterfaceMpSlot:
        return "InterfaceMpSlot";
    case AllocatorType::MaterialEffect:
        return "MaterialEffect";
    case AllocatorType::Permanent:
        return "Permanent";
    case AllocatorType::DlcEffectPreload:
        return "DlcEffectPreload";
    case AllocatorType::DlcItemPreload:
        return "DlcItemPreload";
    case AllocatorType::NumAllocatorTypes:
        return "NumAllocatorTypes";
    default:
        throw std::runtime_error("Invalid AllocatorType enum value passed to to_string()!");
    }
}

static string to_string(ContainerType value)
{
    switch (value)
    {
    case ContainerType::None:
        return "None";
    case ContainerType::Glass:
        return "Glass";
    case ContainerType::EffectsEnv:
        return "EffectsEnv";
    case ContainerType::EffectsPreload:
        return "EffectsPreload";
    case ContainerType::EffectsDlc:
        return "EffectsDlc";
    case ContainerType::MpEffects:
        return "MpEffects";
    case ContainerType::LayerSmall:
        return "LayerSmall";
    case ContainerType::LayerLarge:
        return "LayerLarge";
    case ContainerType::Audio:
        return "Audio";
    case ContainerType::ClothSim:
        return "ClothSim";
    case ContainerType::Decals:
        return "Decals";
    case ContainerType::DecalsPreload:
        return "DecalsPreload";
    case ContainerType::Fsm:
        return "Fsm";
    case ContainerType::Ui:
        return "UI";
    case ContainerType::Env:
        return "Env";
    case ContainerType::Chunk:
        return "Chunk";
    case ContainerType::ChunkPreload:
        return "ChunkPreload";
    case ContainerType::Stitch:
        return "Stitch";
    case ContainerType::World:
        return "World";
    case ContainerType::HumanHead:
        return "HumanHead";
    case ContainerType::Human:
        return "Human";
    case ContainerType::Player:
        return "Player";
    case ContainerType::Items:
        return "Items";
    case ContainerType::ItemsPreload:
        return "ItemsPreload";
    case ContainerType::ItemsMpPreload:
        return "ItemsMpPreload";
    case ContainerType::ItemsDlc:
        return "ItemsDlc";
    case ContainerType::WeaponLarge:
        return "WeaponLarge";
    case ContainerType::WeaponSmall:
        return "WeaponSmall";
    case ContainerType::Skybox:
        return "Skybox";
    case ContainerType::Vehicle:
        return "Vehicle";
    case ContainerType::VoicePersona:
        return "VoicePersona";
    case ContainerType::AlwaysLoadedVoicePersona:
        return "AlwaysLoadedVoicePersona";
    case ContainerType::Foliage:
        return "Foliage";
    case ContainerType::UiPeg:
        return "UiPeg";
    case ContainerType::MaterialEffect:
        return "MaterialEffect";
    case ContainerType::MaterialPreload:
        return "MaterialPreload";
    case ContainerType::SharedBackpack:
        return "SharedBackpack";
    case ContainerType::LandmarkLod:
        return "LandmarkLod";
    case ContainerType::GpsPreload:
        return "GpsPreload";
    case ContainerType::NumContainerTypes:
        return "NumContainerTypes";
    default:
        throw std::runtime_error("Invalid ContainerType enum value passed to to_string()!");
    }
}

//10am 19th

static string to_string(PrimitiveType value)
{
    switch (value)
    {
    case PrimitiveType::None:
        return "None";
    case PrimitiveType::Peg:
        return "Peg";
    case PrimitiveType::Chunk:
        return "Chunk";
    case PrimitiveType::Zone:
        return "Zone";
    case PrimitiveType::Terrain:
        return "Terrain";
    case PrimitiveType::StaticMesh:
        return "StaticMesh";
    case PrimitiveType::CharacterMesh:
        return "CharacterMesh";
    case PrimitiveType::FoliageMesh:
        return "FoliageMesh";
    case PrimitiveType::Material:
        return "Material";
    case PrimitiveType::ClothSim:
        return "ClothSim";
    case PrimitiveType::Vehicle:
        return "Vehicle";
    case PrimitiveType::VehicleAudio:
        return "VehicleAudio";
    case PrimitiveType::Vfx:
        return "Vfx";
    case PrimitiveType::Wavebank:
        return "Wavebank";
    case PrimitiveType::FoleyBank:
        return "FoleyBank";
    case PrimitiveType::MeshMorph:
        return "MeshMorph";
    case PrimitiveType::VoicePersona:
        return "VoicePersona";
    case PrimitiveType::AnimFile:
        return "AnimFile";
    case PrimitiveType::Vdoc:
        return "Vdoc";
    case PrimitiveType::LuaScript:
        return "LuaScript";
    case PrimitiveType::Localization:
        return "Localization";
    case PrimitiveType::TerrainHighLod:
        return "TerrainHighLod";
    case PrimitiveType::LandmarkLod:
        return "LandmarkLod";
    case PrimitiveType::NumPrimitiveTypes:
        return "NumPrimitiveTypes";
    default:
        throw std::runtime_error("Invalid PrimitiveType enum value passed to to_string()!");
    }
}