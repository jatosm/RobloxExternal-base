#pragma once
#include <cstdint>
#include <string>

namespace Offsets {
    inline std::string ClientVersion = "version-5a21b87bbc2641cf";

namespace ClientBuild {
    inline constexpr uintptr_t Timestamp = 0x884CD255;
    inline constexpr uintptr_t ImageSize = 0x922A000;
}

namespace FunctionDescriptor {
    inline constexpr uintptr_t Function = 0x80;
}

namespace BoundFuncDescriptor {
    inline constexpr uintptr_t Function = 0x90;
    inline constexpr uintptr_t Name = 0x8;
}

namespace ByteCode {
    inline constexpr uintptr_t Size = 0x28;
}

namespace TechniqueArray {
    inline constexpr uintptr_t BeginOffset = 0x0;
    inline constexpr uintptr_t EndOffset = 0x8;
    inline constexpr uintptr_t EntryStride = 0x88;
}

namespace MaterialLayer {
    inline constexpr uintptr_t ColorData = 0x26;
    inline constexpr uintptr_t CullMode = 0x10;
    inline constexpr uintptr_t FillModeByte = 0x11;
    inline constexpr uintptr_t Flags2 = 0x20;
    inline constexpr uintptr_t MatFlags = 0x18;
    inline constexpr uintptr_t Param = 0x1C;
    inline constexpr uintptr_t Stride = 0x88;
}

namespace PrimitiveFlags {
    inline constexpr uintptr_t Anchored = 0x2;
    inline constexpr uintptr_t CanCollide = 0x8;
    inline constexpr uintptr_t CanTouch = 0x10;
    inline constexpr uintptr_t CanQuery = 0x20;
}

namespace FastClusterEntity_PrimitiveRecord {
    inline constexpr uintptr_t Row0 = 0x0;
    inline constexpr uintptr_t Row1 = 0xC;
    inline constexpr uintptr_t Row2 = 0x18;
    inline constexpr uintptr_t Stride = 0x30;
    inline constexpr uintptr_t Translation = 0x24;
}

namespace FastClusterEntity_Context {
    inline constexpr uintptr_t PrimitivePoolPtr = 0x198;
}

namespace FastClusterEntity_PrimitivePool {
    inline constexpr uintptr_t ArrayBase = 0x20;
}

namespace FastCluster {
    inline constexpr uintptr_t ClusterNode = 0x188;
    inline constexpr uintptr_t VTableRva = 0x6FC1F40;
    inline constexpr uintptr_t VTableRvaSub = 0x6FC1F20;
    inline constexpr uintptr_t VTableRvaSubAlt = 0x6FC21A8;
    inline constexpr uintptr_t MeshEntityVTableRva = 0x6FC30F8;
}

namespace Primitive {
    inline constexpr uintptr_t Validate = 0x6;
    inline constexpr uintptr_t Position = 0xD4;
    inline constexpr uintptr_t Owner = 0x218;
    inline constexpr uintptr_t Part = 0x218;
    inline constexpr uintptr_t Size = 0x1C4;
    inline constexpr uintptr_t Rotation = 0xB0;
    inline constexpr uintptr_t Material = 0x24E;
         inline constexpr uintptr_t AssemblyLinearVelocity = 0xE0;
         inline constexpr uintptr_t AssemblyAngularVelocity = 0xEC;
         inline constexpr uintptr_t Geometry = 0x210;
         inline constexpr uintptr_t PrimitiveFlags = 0x0;
}

namespace TaskScheduler {
    inline constexpr uintptr_t Pointer = 0x8C9D698;
    inline constexpr uintptr_t StepFunction = 0x29E9210;
    inline constexpr uintptr_t JobStart = 0xC8;
    inline constexpr uintptr_t JobEnd = 0xD0;
    inline constexpr uintptr_t JobName = 0x18;
    inline constexpr uintptr_t MaxFPS = 0xB0;
    inline constexpr uintptr_t MaxFps = 0xB0;
}

namespace DeviceD3D11Gfx {
    inline constexpr uintptr_t VTableRva = 0x6F5E548;
    inline constexpr uintptr_t ContextObjVTableRva = 0x6F5E730;
    inline constexpr uintptr_t SwapChainPtr = 0x80;
    inline constexpr uintptr_t DevicePtr = 0x178;
    inline constexpr uintptr_t ContextObj = 0x1B0;
    inline constexpr uintptr_t ContextObjVTableLen = 0x27;
    inline constexpr uintptr_t ContextObjToContext = 0x18;
    inline constexpr uintptr_t ContextObjDrawSlot = 0x1C;
}

namespace GeometryD3D11 {
    inline constexpr uintptr_t VTableRva = 0x6F5EA50;
}

namespace FastClusterBinding {
    inline constexpr uintptr_t VTableRva = 0x6FC2FD8;
}

namespace FastClusterEntity {
    inline constexpr uintptr_t VTableRva = 0x6FC30B0;
}

namespace RbxTransport {
    inline constexpr uintptr_t AppVTable = 0x6F525C0;
    inline constexpr uintptr_t ClientVTable = 0x6FC3B30;
    inline constexpr uintptr_t AppRTTI = 0x826E460;
    inline constexpr uintptr_t ClientRTTI = 0x82B0128;
}

namespace RemoteEvent {
    inline constexpr uintptr_t BaseVTableRva = 0x6FB57A8;
    inline constexpr uintptr_t DescBaseVTableRva = 0x6FB5540;
    inline constexpr uintptr_t InstanceVTableRva = 0x6F473B0;
    inline constexpr uintptr_t RemoteFuncVTableRva = 0x6FB9CE0;
    inline constexpr uintptr_t VTableRva = 0x6FBA1C8;
    inline constexpr uintptr_t HookSlot7RemoteEventFn = 0x3526660;
    inline constexpr uintptr_t FireServerRva = 0x70C170;
}

namespace ShaderManager {
    inline constexpr uintptr_t VTableRva = 0x6FC0C90;
}

namespace ViewBase {
    inline constexpr uintptr_t VTableRva = 0x6FBF908;
}

namespace RakNet {
    inline constexpr uintptr_t RakPeerVirtualTable = 0x6F5F080;
}

namespace WorldRoot {
    inline constexpr uintptr_t RaycastDescriptorVtableRva = 0x6EB7770;
    inline constexpr uintptr_t RaycastBoundFn = 0x90;
    inline constexpr uintptr_t RaycastDescriptorRva = 0x8494FF0;
    inline constexpr uintptr_t RaycastBoundFnRva = 0xD11E40;
    inline constexpr uintptr_t FindPartOnRayDescriptorRva = 0x8495B90;
    inline constexpr uintptr_t FindPartOnRayBoundFnRva = 0xD1AD70;
    inline constexpr uintptr_t FindPartOnRayWithIgnoreListDescriptorRva = 0x8495C70;
    inline constexpr uintptr_t FindPartOnRayWithIgnoreListBoundFnRva = 0xD1ADF0;
    inline constexpr uintptr_t FindPartOnRayWithWhitelistDescriptorRva = 0x8495D50;
         inline constexpr uintptr_t FindPartOnRayWithWhitelistBoundFnRva = 0xD1AE80;
         inline constexpr uintptr_t ShapecastDescriptorRva = 0x8495270;
         inline constexpr uintptr_t RaycastBoundDesc = RaycastDescriptorRva;
}

namespace Luau {
    inline constexpr uintptr_t luaB_assert = 0x2641500;
    inline constexpr uintptr_t luaB_error = 0x263CBE0;
    inline constexpr uintptr_t luaB_gcinfo = 0x263FD70;
    inline constexpr uintptr_t luaB_getfenv = 0x263E190;
    inline constexpr uintptr_t luaB_getmetatable = 0x263D610;
    inline constexpr uintptr_t luaB_newproxy = 0x2642110;
    inline constexpr uintptr_t luaB_next = 0x2640A50;
    inline constexpr uintptr_t luaB_print = 0x263C780;
    inline constexpr uintptr_t luaB_rawequal = 0x263E470;
    inline constexpr uintptr_t luaB_rawget = 0x263E580;
    inline constexpr uintptr_t luaB_rawlen = 0x263FCD0;
    inline constexpr uintptr_t luaB_rawset = 0x263F010;
    inline constexpr uintptr_t luaB_select = 0x26415B0;
    inline constexpr uintptr_t luaB_setfenv = 0x263E250;
    inline constexpr uintptr_t luaB_setmetatable = 0x263D790;
    inline constexpr uintptr_t luaB_tonumber = 0x263C850;
    inline constexpr uintptr_t luaB_tostring = 0x26420B0;
    inline constexpr uintptr_t luaB_type = 0x263FD90;
    inline constexpr uintptr_t luaB_typeof = 0x2640400;
    inline constexpr uintptr_t tclone = 0x26531F0;
}

namespace Coroutine {
    inline constexpr uintptr_t create = 0x59935D0;
    inline constexpr uintptr_t running = 0x5993E90;
    inline constexpr uintptr_t status = 0x5991B10;
    inline constexpr uintptr_t wrap = 0x5993B70;
    inline constexpr uintptr_t yield = 0x5993E30;
    inline constexpr uintptr_t isyieldable = 0x5993F00;
    inline constexpr uintptr_t finally = 0x5995670;
}

namespace FFlags {
    inline constexpr uintptr_t EnableLoadModule = 0x7FF4F58;
    inline constexpr uintptr_t NextGenReplicatorEnabledWrite4 = 0x7FC0370;
    inline constexpr uintptr_t RenderFastClusterOcclusionCulling4 = 0x7FE7E10;
    inline constexpr uintptr_t CLI52064IxpTxParam = 0x808BEB8;
    inline constexpr uintptr_t DebugRomarkSpeechAssetId = 0x809FEB8;
    inline constexpr uintptr_t AppChatNewChatInputBarIxpLayer = 0x7C2A6F53;
    inline constexpr uintptr_t R20UniverseAccessTime = 0x45003731;
    inline constexpr uintptr_t RealNamesInDisplayNamesExperimentDefinition = 0x82336C41;
    inline constexpr uintptr_t IXPPerformanceControlMLThresholdThreshold = 0x80A1E28;
    inline constexpr uintptr_t NewFriendRequestsBadgeLayer = 0x7C2A6F53;
    inline constexpr uintptr_t DebugLocalTeamTestRccHost = 0x80A1030;
    inline constexpr uintptr_t DebugLocalTeamCreateRccHost = 0x80A1000;
    inline constexpr uintptr_t BlackbirdUrl = 0x4D3E7777;
    inline constexpr uintptr_t DomainAllowListKnownBlacklistRegex = 0x8098588;
    inline constexpr uintptr_t DebugRomarkPlayerTurnoverPattern = 0x8055DE0;
    inline constexpr uintptr_t CaptureServiceStartUploadUniverseId = 0x807F180;
    inline constexpr uintptr_t RCCMonitoring = 0x82397245;
    inline constexpr uintptr_t rbxGreen = 0x42F73734;
    inline constexpr uintptr_t CLI52064IxpRxParam = 0x808BEE8;
    inline constexpr uintptr_t SplitPaneTourStep1CollapsedVideoAssetId = 0x49FB3131;
    inline constexpr uintptr_t PaginatedCarouselExperimentLayer = 0x7C2A6F53;
    inline constexpr uintptr_t SplitPaneTourStep2GamepadVideoAssetId = 0x48FB3231;
    inline constexpr uintptr_t MeshStreamingLODTranscodeDefaultFormat = 0x8067AF8;
    inline constexpr uintptr_t StudioPluginRelay_NewDocking = 0x75396556;
    inline constexpr uintptr_t AssistantMeshGenImageGenModelOverride = 0x78297563;
    inline constexpr uintptr_t V1MenuLanguageSelectionFeatureForcedUserIds = 0x46F73133;
    inline constexpr uintptr_t NewAddFriendsTileIXPLayer = 0x7C2A6F53;
    inline constexpr uintptr_t StudioPublishService = 0x75396556;
    inline constexpr uintptr_t rbxBlue = 0x4BFF3434;
    inline constexpr uintptr_t SplitPaneTourStep3GamepadVideoAssetId = 0x48FC3738;
    inline constexpr uintptr_t OpenTypeFeaturesWhitelist = 0x8031210;
    inline constexpr uintptr_t StudioPluginOTANew = 0x75396556;
    inline constexpr uintptr_t NewChatTabExperimentLayerValue = 0x8A366853;
    inline constexpr uintptr_t ThumbnailGeneratorMaxUploadSizeBytes = 0x42F73032;
    inline constexpr uintptr_t SocialCaptureIXPLayer = 0x7C2A6F53;
    inline constexpr uintptr_t Feature_AccessCookiesWithUrlEnabledSticky = 0x4DFD3635;
    inline constexpr uintptr_t RobloxTelemetryCheckSumSalt = 0x805B810;
    inline constexpr uintptr_t StudioQtitanDocking_NewDocking = 0x81396157;
    inline constexpr uintptr_t LightstepToken = 0x8058020;
    inline constexpr uintptr_t AltTelegrafAddress = 0x43F53533;
    inline constexpr uintptr_t AssetAccessController = 0x75396556;
    inline constexpr uintptr_t DMChangeTrackerBufferSize = 0x46FF3632;
    inline constexpr uintptr_t DMRecorderBatchOutputPath = 0x8025060;
}

namespace Raycast {
    inline constexpr uintptr_t RaycastBoundFn = 0x90;
}

namespace ScriptContext {
    inline constexpr uintptr_t VTable = 0x70271F8;
    inline constexpr uintptr_t Resume = 0x4270490;
    inline constexpr uintptr_t VmWrapper = 0x220;
    inline constexpr uintptr_t VmWrapperBig = 0x440;
    inline constexpr uintptr_t VmWrapper2 = 0x528;
    inline constexpr uintptr_t LuaState = 0x28;
    inline constexpr uintptr_t LuaStateAlt = 0xE8;
}

namespace LuauGlobal {
    inline constexpr uintptr_t totalbytes = 0x50;
    inline constexpr uintptr_t GCthreshold = 0x48;
    inline constexpr uintptr_t strt_size = 0xC;
    inline constexpr uintptr_t strt_hash = 0x0;
    inline constexpr uintptr_t gcopages = 0x58;
    inline constexpr uintptr_t gray = 0x10;
    inline constexpr uintptr_t weak = 0x18;
    inline constexpr uintptr_t grayagain = 0x20;
    inline constexpr uintptr_t gcstate = 0x39;
    inline constexpr uintptr_t gcgoal = 0x3C;
    inline constexpr uintptr_t gcpause = 0x3C;
    inline constexpr uintptr_t gcstepsize = 0x40;
    inline constexpr uintptr_t gcstepmul = 0x44;
    inline constexpr uintptr_t mainthread = 0x70;
    inline constexpr uintptr_t gcopages_large = 0x4A8;
    inline constexpr uintptr_t page_next_all = 0x8;
    inline constexpr uintptr_t page_next_free = 0x18;
    inline constexpr uintptr_t sweepgcopage = 0x68;
    inline constexpr uintptr_t dummynode = 0x67352E8;
}

namespace LuaState {
    inline constexpr uintptr_t Global = 0x38;
    inline constexpr uintptr_t Tt = 0x0;
    inline constexpr uintptr_t Marked = 0x1;
    inline constexpr uintptr_t MemCat = 0x2;
    inline constexpr uintptr_t Status = 0x3;
    inline constexpr uintptr_t ActiveMemCat = 0x4;
    inline constexpr uintptr_t IsActive = 0x5;
    inline constexpr uintptr_t SingleStep = 0x6;
    inline constexpr uintptr_t Ci = 0x8;
    inline constexpr uintptr_t StackLast = 0x10;
    inline constexpr uintptr_t Top = 0x20;
    inline constexpr uintptr_t Base = 0x28;
    inline constexpr uintptr_t Stack = 0x30;
    inline constexpr uintptr_t OpenUpval = 0x38;
    inline constexpr uintptr_t StackSize = 0x40;
    inline constexpr uintptr_t SizeCi = 0x44;
    inline constexpr uintptr_t GcList = 0x50;
    inline constexpr uintptr_t Gt = 0x68;
    inline constexpr uintptr_t EndCi = 0x70;
    inline constexpr uintptr_t ExtraSpace = 0x80;
    inline constexpr uintptr_t Userdata = 0x80;
}

namespace Instance {
    inline constexpr uintptr_t SetParent = 0x842510;
    inline constexpr uintptr_t This = 0x8;
    inline constexpr uintptr_t NameContainer = 0x68;
    inline constexpr uintptr_t Name = 0x0;
    inline constexpr uintptr_t ChildrenStart = 0x70;
    inline constexpr uintptr_t ChildrenEnd = 0x8;
    inline constexpr uintptr_t ChildrenStride = 0x10;
    inline constexpr uintptr_t Parent = 0x60;
    inline constexpr uintptr_t ClassDescriptor = 0x18;
    inline constexpr uintptr_t ClassName = 0x8;
    inline constexpr uintptr_t ClassBase = 0x1B0;
    inline constexpr uintptr_t FindFirstChild = 0x1481460;
    inline constexpr uintptr_t FindFirstChildOfClass = 0x1C9CCB0;
    inline constexpr uintptr_t FindFirstChildWhichIsA = 0x1481410;
    inline constexpr uintptr_t GetAttribute = 0x1CA65F0;
    inline constexpr uintptr_t GetChildren = 0xEA1690;
    inline constexpr uintptr_t FindFirstAncestorWhichIsA = 0x1481700;
    inline constexpr uintptr_t Creator_create = 0x1C8B370;
}

namespace Alloc {
    inline constexpr uintptr_t Malloc = 0x4A037D0;
}

namespace Functions {
    inline constexpr uintptr_t GlobalPrint = 0x42774C0;
    inline constexpr uintptr_t Print = 0x42774C0;
    inline constexpr uintptr_t GlobalTick = 0x4278D60;
    inline constexpr uintptr_t GlobalWait = 0x4279520;
    inline constexpr uintptr_t GlobalDelay = 0x4276110;
    inline constexpr uintptr_t GlobalSpawn = 0x4278080;
    inline constexpr uintptr_t PrintIdentity = 0x42774E0;
    inline constexpr uintptr_t GlobalLoadstring = 0x4276910;
    inline constexpr uintptr_t GlobalWarn = 0x4279720;
    inline constexpr uintptr_t GlobalRequire = 0x42775E0;
    inline constexpr uintptr_t GlobalCollectGarbage = 0x4275870;
    inline constexpr uintptr_t GlobalGetFenv = 0x4279F20;
    inline constexpr uintptr_t GlobalSetFenv = 0x427A060;
    inline constexpr uintptr_t GlobalTime = 0x4278DE0;
    inline constexpr uintptr_t GlobalElapsedTime = 0x4276890;
    inline constexpr uintptr_t GlobalSettings = 0x4277BB0;
    inline constexpr uintptr_t GlobalUserSettings = 0x4278FE0;
    inline constexpr uintptr_t GlobalPluginManager = 0x4276FB0;
    inline constexpr uintptr_t GlobalStats = 0x42787F0;
    inline constexpr uintptr_t GlobalVersion = 0x4279430;
    inline constexpr uintptr_t Shapecast = 0xD13BE0;
    inline constexpr uintptr_t Clone = 0x14813C0;
    inline constexpr uintptr_t Destroy = 0x14813E0;
}

namespace Misc {
    inline constexpr uintptr_t PrintIdentity = 0x42774E0;
    inline constexpr uintptr_t StringLength = 0x10;
    inline constexpr uintptr_t Adornee = 0xD8;
    inline constexpr uintptr_t Value = 0xA0;
    inline constexpr uintptr_t AnimationId = 0xA8;
}

namespace LuaCoroutine {
    inline constexpr uintptr_t Create = 0x59935D0;
    inline constexpr uintptr_t Running = 0x5993E90;
    inline constexpr uintptr_t Status = 0x5991B10;
    inline constexpr uintptr_t Wrap = 0x5993B70;
    inline constexpr uintptr_t Yield = 0x5993E30;
    inline constexpr uintptr_t Isyieldable = 0x5993F00;
    inline constexpr uintptr_t Finally = 0x5995670;
}

namespace LuaB {
    inline constexpr uintptr_t LuaBAssert = 0x2641500;
    inline constexpr uintptr_t LuaBError = 0x263CBE0;
    inline constexpr uintptr_t LuaBGcinfo = 0x263FD70;
    inline constexpr uintptr_t LuaBGetfenv = 0x263E190;
    inline constexpr uintptr_t LuaBGetmetatable = 0x263D610;
    inline constexpr uintptr_t LuaBNext = 0x2640A50;
    inline constexpr uintptr_t LuaBNewproxy = 0x2642110;
    inline constexpr uintptr_t LuaBPrint = 0x263C780;
    inline constexpr uintptr_t LuaBRawequal = 0x263E470;
    inline constexpr uintptr_t LuaBRawget = 0x263E580;
    inline constexpr uintptr_t LuaBRawset = 0x263F010;
    inline constexpr uintptr_t LuaBRawlen = 0x263FCD0;
    inline constexpr uintptr_t LuaBSelect = 0x26415B0;
    inline constexpr uintptr_t LuaBSetfenv = 0x263E250;
    inline constexpr uintptr_t LuaBSetmetatable = 0x263D790;
    inline constexpr uintptr_t LuaBTonumber = 0x263C850;
    inline constexpr uintptr_t LuaBTostring = 0x26420B0;
    inline constexpr uintptr_t LuaBType = 0x263FD90;
    inline constexpr uintptr_t LuaBTypeof = 0x2640400;
}

namespace VisualEngine {
    inline constexpr uintptr_t Pointer = 0x879A218;
    inline constexpr uintptr_t ViewMatrix = 0x1B0;
    inline constexpr uintptr_t RenderView = 0xC40;
    inline constexpr uintptr_t FakeDataModel = 0xAF0;
    inline constexpr uintptr_t Dimensions = 0x0;
}

namespace FakeDataModel {
    inline constexpr uintptr_t Pointer = 0x8D06600;
    inline constexpr uintptr_t RealDataModel = 0x1F8;
}

namespace PlayerConfigurer {
    inline constexpr uintptr_t Pointer = 0x8BE0330;
}

namespace Descriptor {
    inline constexpr uintptr_t Name = 0x0;
}

namespace ClassDescriptor {
    inline constexpr uintptr_t ClassName = 0x8;
    inline constexpr uintptr_t PropertyDescriptors = 0x40;
    inline constexpr uintptr_t EventDescriptors = 0x88;
    inline constexpr uintptr_t FunctionDescriptors = 0xD0;
    inline constexpr uintptr_t Creator = 0x60;
}

namespace PropertyDescriptor {
    inline constexpr uintptr_t TType = 0x70;
}

namespace Value {
    inline constexpr uintptr_t Value = 0xA0;
}

namespace DataModel {
    inline constexpr uintptr_t PlaceId = 0x180;
    inline constexpr uintptr_t GameId = 0x178;
    inline constexpr uintptr_t CreatorId = 0x170;
    inline constexpr uintptr_t GameLoaded = 0x588;
    inline constexpr uintptr_t JobId = 0x108;
    inline constexpr uintptr_t Workspace = 0x148;
    inline constexpr uintptr_t ScriptContext = 0x440;
    inline constexpr uintptr_t ServerIP = 0x570;
    inline constexpr uintptr_t ToRenderView1 = 0x1B8;
    inline constexpr uintptr_t ToRenderView2 = 0x8;
    inline constexpr uintptr_t ToRenderView3 = 0x28;
}

namespace RunService {
    inline constexpr uintptr_t HeartbeatTask = 0x1E0;
    inline constexpr uintptr_t HeartbeatFPS = 0xE8;
}

namespace RenderJob {
    inline constexpr uintptr_t RenderView = 0x1E0;
    inline constexpr uintptr_t FakeDataModel = 0x38;
    inline constexpr uintptr_t RealDataModel = 0x1F0;
}

namespace RenderView {
    inline constexpr uintptr_t VisualEngine = 0x18;
    inline constexpr uintptr_t LightingValid = 0x278;
    inline constexpr uintptr_t SkyValid = 0x2DC;
    inline constexpr uintptr_t DeviceD3D11 = 0x8;
}

namespace Workspace {
    inline constexpr uintptr_t World = 0x3F8;
    inline constexpr uintptr_t ReadOnlyGravity = 0x9C0;
    inline constexpr uintptr_t DistributedGameTime = 0x4C0;
    inline constexpr uintptr_t CurrentCamera = 0x4A0;
}

namespace World {
    inline constexpr uintptr_t Gravity = 0x218;
    inline constexpr uintptr_t FallenPartsDestroyHeight = 0x20C;
    inline constexpr uintptr_t AirProperties = 0x230;
    inline constexpr uintptr_t Primitives = 0x2A0;
}

namespace AirProperties {
    inline constexpr uintptr_t AirDensity = 0x18;
    inline constexpr uintptr_t GlobalWind = 0x3C;
}

namespace Terrain {
    inline constexpr uintptr_t WaterReflectance = 0x1E0;
    inline constexpr uintptr_t GrassLength = 0x1D8;
    inline constexpr uintptr_t WaterTransparency = 0x1CC;
    inline constexpr uintptr_t WaterWaveSize = 0x1E8;
    inline constexpr uintptr_t WaterWaveSpeed = 0x1EC;
    inline constexpr uintptr_t WaterColor = 0x1C8;
    inline constexpr uintptr_t MaterialColors = 0x4A0;
}

namespace MaterialColors {
    inline constexpr uintptr_t Asphalt = 0x30;
    inline constexpr uintptr_t Basalt = 0x27;
    inline constexpr uintptr_t Brick = 0xF;
    inline constexpr uintptr_t Cobblestone = 0x33;
    inline constexpr uintptr_t Concrete = 0xC;
    inline constexpr uintptr_t CrackedLava = 0x2D;
    inline constexpr uintptr_t Glacier = 0x1B;
    inline constexpr uintptr_t Grass = 0x6;
    inline constexpr uintptr_t Ground = 0x2A;
    inline constexpr uintptr_t Ice = 0x36;
    inline constexpr uintptr_t LeafyGrass = 0x39;
    inline constexpr uintptr_t Limestone = 0x3F;
    inline constexpr uintptr_t Mud = 0x24;
    inline constexpr uintptr_t Pavement = 0x42;
    inline constexpr uintptr_t Rock = 0x18;
    inline constexpr uintptr_t Salt = 0x3C;
    inline constexpr uintptr_t Sand = 0x12;
    inline constexpr uintptr_t Sandstone = 0x21;
    inline constexpr uintptr_t Slate = 0x9;
    inline constexpr uintptr_t Snow = 0x1E;
    inline constexpr uintptr_t WoodPlanks = 0x15;
}

namespace Sound {
    inline constexpr uintptr_t SoundId = 0xB0;
    inline constexpr uintptr_t RollOffMaxDistance = 0x108;
    inline constexpr uintptr_t RollOffMinDistance = 0x10C;
    inline constexpr uintptr_t PlaybackSpeed = 0x104;
    inline constexpr uintptr_t Volume = 0x118;
    inline constexpr uintptr_t SoundGroup = 0xD0;
    inline constexpr uintptr_t Looped = 0x125;
    inline constexpr uintptr_t IsPlaying = 0x128;
}

namespace SpawnLocation {
    inline constexpr uintptr_t Neutral = 0x1DA;
    inline constexpr uintptr_t AllowTeamChangeOnTouch = 0x1D8;
    inline constexpr uintptr_t Enabled = 0x1D9;
}

namespace SurfaceAppearance {
    inline constexpr uintptr_t ColorMap = 0xB0;
    inline constexpr uintptr_t EmissiveMaskContent = 0xE0;
    inline constexpr uintptr_t EmissiveStrength = 0x1DC;
    inline constexpr uintptr_t MetalnessMap = 0x110;
    inline constexpr uintptr_t NormalMap = 0x140;
    inline constexpr uintptr_t RoughnessMap = 0x170;
}

namespace ParticleEmitter {
    inline constexpr uintptr_t Brightness = 0x95;
    inline constexpr uintptr_t LightEmission = 0x220;
    inline constexpr uintptr_t LightInfluence = 0x224;
    inline constexpr uintptr_t Texture = 0x1A8;
    inline constexpr uintptr_t ZOffset = 0x24C;
    inline constexpr uintptr_t Lifetime = 0x1DC;
    inline constexpr uintptr_t Rate = 0x230;
    inline constexpr uintptr_t Rotation = 0x1EC;
    inline constexpr uintptr_t RotSpeed = 0x1E4;
    inline constexpr uintptr_t Speed = 0x1F4;
    inline constexpr uintptr_t SpreadAngle = 0x1FC;
    inline constexpr uintptr_t Acceleration = 0x1C8;
    inline constexpr uintptr_t Drag = 0x208;
    inline constexpr uintptr_t TimeScale = 0x244;
    inline constexpr uintptr_t VelocityInheritance = 0x248;
}

namespace Beam {
    inline constexpr uintptr_t Brightness = 0x168;
    inline constexpr uintptr_t LightEmission = 0x174;
    inline constexpr uintptr_t LightInfluence = 0x178;
    inline constexpr uintptr_t Texture = 0x128;
    inline constexpr uintptr_t TextureLength = 0x184;
    inline constexpr uintptr_t TextureSpeed = 0x18C;
    inline constexpr uintptr_t ZOffset = 0x198;
    inline constexpr uintptr_t Attachment0 = 0x148;
    inline constexpr uintptr_t Attachment1 = 0x158;
    inline constexpr uintptr_t CurveSize0 = 0x16C;
    inline constexpr uintptr_t CurveSize1 = 0x170;
    inline constexpr uintptr_t Width0 = 0x190;
    inline constexpr uintptr_t Width1 = 0x194;
}

namespace Players {
    inline constexpr uintptr_t LocalPlayer = 0x118;
}

namespace Player {
    inline constexpr uintptr_t UserId = 0xB8;
    inline constexpr uintptr_t HealthDisplayDistance = 0x3A0;
    inline constexpr uintptr_t NameDisplayDistance = 0x3B0;
    inline constexpr uintptr_t ModelInstance = 0x2A0;
    inline constexpr uintptr_t Team = 0x2E0;
    inline constexpr uintptr_t LocaleId = 0x100;
    inline constexpr uintptr_t AccountAge = 0x128;
    inline constexpr uintptr_t MinZoomDistance = 0x374;
    inline constexpr uintptr_t MaxZoomDistance = 0x370;
    inline constexpr uintptr_t CameraMode = 0x378;
    inline constexpr uintptr_t Mouse = 0x11F0;
}

namespace Humanoid {
    inline constexpr uintptr_t MaxHealth = 0x19C;
    inline constexpr uintptr_t Health = 0x184;
         inline constexpr uintptr_t Walkspeed = 0x1C4;
         inline constexpr uintptr_t WalkSpeed = 0x1C4;
         inline constexpr uintptr_t WalkspeedCheck = 0x39C;
         inline constexpr uintptr_t WalkSpeedCheck = WalkspeedCheck;
    inline constexpr uintptr_t JumpPower = 0x198;
         inline constexpr uintptr_t JumpHeight = 0x194;
         inline constexpr uintptr_t HipHeight = 0x188;
    inline constexpr uintptr_t MaxSlopeAngle = 0x1A0;
    inline constexpr uintptr_t SeatPart = 0xF0;
    inline constexpr uintptr_t HumanoidRootPart = 0x460;
    inline constexpr uintptr_t HealthDisplayDistance = 0x17C;
    inline constexpr uintptr_t NameDisplayDistance = 0x1A4;
    inline constexpr uintptr_t NameOcclusion = 0x1A8;
    inline constexpr uintptr_t DisplayName = 0xA0;
    inline constexpr uintptr_t RigType = 0x1B4;
    inline constexpr uintptr_t MoveDirection = 0x134;
    inline constexpr uintptr_t Jump = 0x1D0;
    inline constexpr uintptr_t Sit = 0x1D3;
    inline constexpr uintptr_t PlatformStand = 0x1D2;
    inline constexpr uintptr_t UseJumpPower = 0x1D6;
    inline constexpr uintptr_t AutomaticScalingEnabled = 0x1CA;
    inline constexpr uintptr_t BreakJointsOnDeath = 0x1CC;
    inline constexpr uintptr_t EvaluateStateMachine = 0x1CD;
    inline constexpr uintptr_t RequiresNeck = 0x1D3;
    inline constexpr uintptr_t AutoJumpEnabled = 0x1C8;
    inline constexpr uintptr_t AutoRotate = 0x1C9;
    inline constexpr uintptr_t MoveToPoint = 0x158;
    inline constexpr uintptr_t MoveToPart = 0x100;
    inline constexpr uintptr_t FloorMaterial = 0x174;
    inline constexpr uintptr_t TargetPoint = 0x140;
}

namespace Seat {
    inline constexpr uintptr_t Occupant = 0x200;
}

namespace VehicleSeat {
    inline constexpr uintptr_t Occupant = 0x1F8;
    inline constexpr uintptr_t MaxSpeed = 0x210;
    inline constexpr uintptr_t SteerFloat = 0x214;
    inline constexpr uintptr_t ThrottleFloat = 0x218;
    inline constexpr uintptr_t Torque = 0x21C;
    inline constexpr uintptr_t TurnSpeed = 0x220;
}

namespace StatsItem {
    inline constexpr uintptr_t Value = 0x98;
}

namespace Tool {
    inline constexpr uintptr_t Tooltip = 0x450;
    inline constexpr uintptr_t TextureId = 0x348;
    inline constexpr uintptr_t Grip = 0x494;
    inline constexpr uintptr_t GripForward = 0x490;
    inline constexpr uintptr_t GripRight = 0x478;
    inline constexpr uintptr_t GripUp = 0x484;
    inline constexpr uintptr_t Enabled = 0x4A1;
    inline constexpr uintptr_t CanBeDropped = 0x4A0;
    inline constexpr uintptr_t ManualActivationOnly = 0x4A2;
    inline constexpr uintptr_t RequiresHandle = 0x4A3;
}

namespace Clothing {
    inline constexpr uintptr_t Template = 0xE8;
}

namespace CharacterMesh {
    inline constexpr uintptr_t BaseTextureId = 0xB0;
    inline constexpr uintptr_t OverlayTextureId = 0x110;
    inline constexpr uintptr_t MeshId = 0xE0;
    inline constexpr uintptr_t BodyPart = 0x130;
}

namespace Camera {
    inline constexpr uintptr_t Rotation = 0xC0;
    inline constexpr uintptr_t CFrame = 0xC0;
    inline constexpr uintptr_t Position = 0xE4;
    inline constexpr uintptr_t CameraSubject = 0xB0;
    inline constexpr uintptr_t CameraType = 0x0;
    inline constexpr uintptr_t ViewportInt16 = 0x0;
    inline constexpr uintptr_t FieldOfView = 0x128;
    inline constexpr uintptr_t ImagePlaneDepth = 0x2D4;
}

namespace BasePart {
    inline constexpr uintptr_t Primitive = 0x178;
    inline constexpr uintptr_t Transparency = 0x118;
    inline constexpr uintptr_t Massless = 0x122;
    inline constexpr uintptr_t CastShadow = 0x120;
    inline constexpr uintptr_t Locked = 0x121;
    inline constexpr uintptr_t Reflectance = 0xF4;
    inline constexpr uintptr_t ClusterNode = 0x180;
    inline constexpr uintptr_t Shape = 0x0;
}

namespace MeshPart {
    inline constexpr uintptr_t MeshId = 0x2F8;
    inline constexpr uintptr_t Texture = 0x328;
    inline constexpr uintptr_t TextureId = 0x328;
    inline constexpr uintptr_t MeshSize = 0x210;
}

namespace Model {
    inline constexpr uintptr_t PrimaryPart = 0x240;
    inline constexpr uintptr_t Scale = 0x12C;
}

namespace SpecialMesh {
    inline constexpr uintptr_t Scale = 0xAC;
    inline constexpr uintptr_t MeshId = 0xE0;
}

namespace Attachment {
    inline constexpr uintptr_t Position = 0xAC;
}

namespace UnionOperation {
    inline constexpr uintptr_t AssetId = 0x2F8;
}

namespace MouseService {
    inline constexpr uintptr_t InputObject = 0xD8;
    inline constexpr uintptr_t InputObject2 = 0xE8;
    inline constexpr uintptr_t MousePosition = 0xBC;
}

namespace PlayerMouse {
    inline constexpr uintptr_t Icon = 0xB0;
    inline constexpr uintptr_t Workspace = 0x138;
}

namespace GuiObject {
    inline constexpr uintptr_t Position = 0x4D8;
    inline constexpr uintptr_t Size = 0x4F8;
    inline constexpr uintptr_t Visible = 0x575;
    inline constexpr uintptr_t Image = 0x9A8;
    inline constexpr uintptr_t Text = 0xE08;
    inline constexpr uintptr_t RichText = 0xBA0;
    inline constexpr uintptr_t TextColor3 = 0xEB8;
    inline constexpr uintptr_t Rotation = 0xD0;
    inline constexpr uintptr_t BackgroundTransparency = 0x514;
    inline constexpr uintptr_t BorderColor3 = 0x514;
    inline constexpr uintptr_t BackgroundColor3 = 0x508;
}

namespace Frame {
    inline constexpr uintptr_t FramePositionOffsetX = 0x4DC;
    inline constexpr uintptr_t FramePositionOffsetY = 0x4E4;
}

namespace GuiBase2D {
    inline constexpr uintptr_t AbsoluteRotation = 0xD0;
    inline constexpr uintptr_t AbsoluteSize = 0xFC;
    inline constexpr uintptr_t AbsolutePosition = 0xF4;
}

namespace Textures {
    inline constexpr uintptr_t Decal_Texture = 0x1C8;
    inline constexpr uintptr_t Texture_Texture = 0x1C8;
}

namespace Lighting {
    inline constexpr uintptr_t ClockTime = 0xB0;
    inline constexpr uintptr_t Brightness = 0x100;
    inline constexpr uintptr_t EnvironmentDiffuseScale = 0x104;
    inline constexpr uintptr_t EnvironmentSpecularScale = 0x108;
    inline constexpr uintptr_t FogStart = 0x118;
    inline constexpr uintptr_t FogEnd = 0x114;
    inline constexpr uintptr_t FogColor = 0x14C;
    inline constexpr uintptr_t Ambient = 0xB8;
    inline constexpr uintptr_t OutdoorAmbient = 0xE8;
    inline constexpr uintptr_t ColorShift_Top = 0xD0;
    inline constexpr uintptr_t ColorShift_Bottom = 0xC4;
    inline constexpr uintptr_t ExposureCompensation = 0x10C;
    inline constexpr uintptr_t GeographicLatitude = 0x11C;
    inline constexpr uintptr_t GradientTop = 0x138;
    inline constexpr uintptr_t LightColor = 0x144;
    inline constexpr uintptr_t GradientBottom = 0x178;
    inline constexpr uintptr_t GlobalShadows = 0x12C;
    inline constexpr uintptr_t MoonPosition = 0x16C;
    inline constexpr uintptr_t SunPosition = 0x160;
         inline constexpr uintptr_t Source = 0x15C;
         inline constexpr uintptr_t Sky = 0x1B0;
         inline constexpr uintptr_t LightDirection = 0x0;
         inline constexpr uintptr_t ShadowSoftness = 0x0;
}

namespace Sky {
    inline constexpr uintptr_t SkyboxBk = 0xE0;
    inline constexpr uintptr_t SkyboxDn = 0x110;
    inline constexpr uintptr_t SkyboxFt = 0x140;
    inline constexpr uintptr_t SkyboxLf = 0x170;
    inline constexpr uintptr_t SkyboxRt = 0x1A0;
    inline constexpr uintptr_t SkyboxUp = 0x1D0;
    inline constexpr uintptr_t SunAngularSize = 0x234;
    inline constexpr uintptr_t MoonAngularSize = 0x22C;
    inline constexpr uintptr_t SunTextureId = 0x200;
    inline constexpr uintptr_t MoonTextureId = 0xB0;
    inline constexpr uintptr_t SkyboxOrientation = 0x220;
}

namespace Atmosphere {
    inline constexpr uintptr_t Density = 0xB8;
    inline constexpr uintptr_t Offset = 0xC4;
    inline constexpr uintptr_t Color = 0xA0;
    inline constexpr uintptr_t Glare = 0xBC;
    inline constexpr uintptr_t Haze = 0xC0;
}

namespace BloomEffect {
    inline constexpr uintptr_t Intensity = 0xA0;
    inline constexpr uintptr_t Size = 0xA4;
    inline constexpr uintptr_t Threshold = 0xA8;
    inline constexpr uintptr_t Enabled = 0x98;
}

namespace DepthOfFieldEffect {
    inline constexpr uintptr_t FocusDistance = 0xA4;
    inline constexpr uintptr_t FarIntensity = 0xA0;
    inline constexpr uintptr_t NearIntensity = 0xAC;
    inline constexpr uintptr_t InFocusRadius = 0xA8;
    inline constexpr uintptr_t Enabled = 0x98;
}

namespace SunRaysEffect {
    inline constexpr uintptr_t Intensity = 0xA0;
    inline constexpr uintptr_t Spread = 0xA4;
    inline constexpr uintptr_t Enabled = 0x98;
}

namespace ColorCorrectionEffect {
    inline constexpr uintptr_t Brightness = 0xAC;
    inline constexpr uintptr_t Contrast = 0xAC;
    inline constexpr uintptr_t Enabled = 0x98;
}

namespace ColorGradingEffect {
    inline constexpr uintptr_t Enabled = 0x98;
}

namespace BlurEffect {
    inline constexpr uintptr_t Size = 0xA0;
    inline constexpr uintptr_t Enabled = 0x98;
}

namespace ProximityPrompt {
    inline constexpr uintptr_t ActionText = 0x98;
    inline constexpr uintptr_t ObjectText = 0xB8;
    inline constexpr uintptr_t HoldDuration = 0x108;
    inline constexpr uintptr_t MaxActivationDistance = 0x110;
    inline constexpr uintptr_t Enabled = 0x98;
    inline constexpr uintptr_t RequiresLineOfSight = 0x11F;
}

namespace ClickDetector {
    inline constexpr uintptr_t MaxActivationDistance = 0xD0;
    inline constexpr uintptr_t MouseIcon = 0xB0;
    inline constexpr uintptr_t CursorIcon = 0xB0;
}

namespace DragDetector {
    inline constexpr uintptr_t ReferenceInstance = 0x1B8;
    inline constexpr uintptr_t MaxActivationDistance = 0xD0;
    inline constexpr uintptr_t MaxDragAngle = 0x270;
    inline constexpr uintptr_t MaxDragTranslation = 0x234;
    inline constexpr uintptr_t MinDragAngle = 0x27C;
    inline constexpr uintptr_t MinDragTranslation = 0x240;
    inline constexpr uintptr_t ActivatedCursorIcon = 0x188;
    inline constexpr uintptr_t CursorIcon = 0xB0;
    inline constexpr uintptr_t MaxForce = 0x274;
    inline constexpr uintptr_t MaxTorque = 0x278;
    inline constexpr uintptr_t Responsiveness = 0x288;
}

namespace Animator {
    inline constexpr uintptr_t ActiveAnimations = 0x900;
}

namespace LocalScript {
    inline constexpr uintptr_t GUID = 0xB8;
    inline constexpr uintptr_t Hash = 0x188;
}

namespace ModuleScript {
    inline constexpr uintptr_t GUID = 0xB8;
}

namespace Script {
    inline constexpr uintptr_t GUID = 0xB8;
    inline constexpr uintptr_t Hash = 0x188;
}

namespace DeviceD3D11 {
    inline constexpr uintptr_t VTableRva = 0x6F5E548;
    inline constexpr uintptr_t Context = 0x180;
    inline constexpr uintptr_t ContextPtr = 0x180;
    inline constexpr uintptr_t DevicePtr = 0x178;
    inline constexpr uintptr_t SwapChainPtr = 0x80;
    inline constexpr uintptr_t ContextObj = 0x1B0;
}

namespace EngineWallCheck {
    inline constexpr uintptr_t RaycastDescriptorRva = 0x8494FF0;
    inline constexpr uintptr_t RealWrapperRva = 0xD11E40;
    inline constexpr uintptr_t BoundFnOffset = 0xB0;
}

namespace Luau_lua_State {
    inline constexpr uintptr_t L_global_offset = 0x38;
}

namespace LiveThreadRef {
    inline constexpr uintptr_t Thread = 0x28;
}

namespace Reflection {
    inline constexpr uintptr_t NameRegistry = 0x83E4B78;
    inline constexpr uintptr_t CreatorTable = 0x8C87F58;
    inline constexpr uintptr_t TableStart = 0x0;
    inline constexpr uintptr_t TableEnd = 0x8;
    inline constexpr uintptr_t TableEmpty = 0x20;
}

namespace FFlagList {
    inline constexpr uintptr_t Pointer = 0x83EBA48;
    inline constexpr uintptr_t ToFlag = 0x30;
    inline constexpr uintptr_t ToValue = 0x20;
}
}
