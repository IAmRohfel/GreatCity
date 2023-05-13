#ifndef GC_APPLICATION_CORE_GENERIC_PLATFORM_KEY_CODE_H
#define GC_APPLICATION_CORE_GENERIC_PLATFORM_KEY_CODE_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum GCKeyCode
	{
#ifdef GC_PLATFORM_WINDOWS
		GCKeyCode_Escape = 0x1B,
		GCKeyCode_Left = 0x25,
		GCKeyCode_Up = 0x26,
		GCKeyCode_Right = 0x27,
		GCKeyCode_Down = 0x28,
		GCKeyCode_PrintScreen = 0x2C,

		GCKeyCode_0 = 0x30,
		GCKeyCode_1 = 0x31,
		GCKeyCode_2 = 0x32,
		GCKeyCode_3 = 0x33,
		GCKeyCode_4 = 0x34,
		GCKeyCode_5 = 0x35,
		GCKeyCode_6 = 0x36,
		GCKeyCode_7 = 0x37,
		GCKeyCode_8 = 0x38,
		GCKeyCode_9 = 0x39,

		GCKeyCode_A = 0x41,
		GCKeyCode_B = 0x42,
		GCKeyCode_C = 0x43,
		GCKeyCode_D = 0x44,
		GCKeyCode_E = 0x45,
		GCKeyCode_F = 0x46,
		GCKeyCode_G = 0x47,
		GCKeyCode_H = 0x48,
		GCKeyCode_I = 0x49,
		GCKeyCode_J = 0x4A,
		GCKeyCode_K = 0x4B,
		GCKeyCode_L = 0x4C,
		GCKeyCode_M = 0x4D,
		GCKeyCode_N = 0x4E,
		GCKeyCode_O = 0x4F,
		GCKeyCode_P = 0x50,
		GCKeyCode_Q = 0x51,
		GCKeyCode_R = 0x52,
		GCKeyCode_S = 0x53,
		GCKeyCode_T = 0x54,
		GCKeyCode_U = 0x55,
		GCKeyCode_V = 0x56,
		GCKeyCode_W = 0x57,
		GCKeyCode_X = 0x58,
		GCKeyCode_Y = 0x59,
		GCKeyCode_Z = 0x5A,

		GCKeyCode_LeftShift = 0xA0,
		GCKeyCode_RightShift = 0xA1,
		GCKeyCode_LeftControl = 0xA2,
		GCKeyCode_RightControl = 0xA3,
		GCKeyCode_LeftAlt	= 0xA4,
		GCKeyCode_RightAlt = 0xA5
#endif
	} GCKeyCode;

#ifdef __cplusplus
}
#endif

#endif