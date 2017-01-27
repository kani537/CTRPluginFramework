#include "types.h"
#include "cheats.hpp"

namespace CTRPluginFramework
{
	#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
	#define BYTE_TO_BINARY(byte)  \
	  (byte & 0x80 ? '1' : '0'), \
	  (byte & 0x40 ? '1' : '0'), \
	  (byte & 0x20 ? '1' : '0'), \
	  (byte & 0x10 ? '1' : '0'), \
	  (byte & 0x08 ? '1' : '0'), \
	  (byte & 0x04 ? '1' : '0'), \
	  (byte & 0x02 ? '1' : '0'), \
	  (byte & 0x01 ? '1' : '0') 
    #define READU32(x) *(u32 *)(x)
	#define READU16(x) *(u16 *)(x)
    #define READU8(x) *(u8 *)(x)

    #define WRITEU8(a, v) 	*(u8 *)(a) = (u8)v
    #define WRITEU16(a, v) 	*(u16 *)(a) = (u16)v
    #define WRITEU32(a, v) 	*(u32 *)(a) = (u32)v 

	static u32	basePointer = 0x005A2E3C;

	void	MaxHeart(MenuEntry *entry)
	{
		WRITEU16(0x058799A, 0x140);
	}

	void	MaxMagic(MenuEntry *entry)
	{
		
		WRITEU8(0x05879A8, 0x00000001);
		WRITEU8(0x058799F, 0x00000060);
	}

	void	RefillHeart(MenuEntry *entry)
	{
		if (Touch::IsDown())
			WRITEU16(0x058799C, 0x140);
	}

	void	RefillLargeMagicbar(MenuEntry *entry)
	{
		if (Touch::IsDown())
			if (READU8(0x05879A8) == 1)
				WRITEU8(0x058799F, 0x60);
			else
				WRITEU8(0x058799F, 0x30);
	}

	void	InfiniteArrows(MenuEntry *entry)
	{
		WRITEU8(0x0587A01, 0x63);
	}

	void	InfiniteNuts(MenuEntry *entry)
	{
		WRITEU8(0x05879FF, 0x63);
	}

	void	InfiniteStick(MenuEntry *entry)
	{
		WRITEU8(0x05879FE, 0x63);
	}

	void	InfiniteBomb(MenuEntry *entry)
	{
		WRITEU8(0x0587A00, 0x63);
		WRITEU8(0x08001AB4, 0x63);
	}

	void	InfiniteBombchu(MenuEntry *entry)
	{
		WRITEU8(0x0587A06, 0x63);
		WRITEU8(0x08001AD0, 0x63);
	}

	void	InfiniteSlingshot(MenuEntry *entry)
	{
		WRITEU8(0x0587A04, 0x63);
	}

	void	Skulltulas(MenuEntry *entry)
	{
		WRITEU8(0x0587A40, 0x64);
		WRITEU8(0x05c2280, 0x64);
	}

	void	MaxRupees(MenuEntry *entry)
	{
		WRITEU16(0x05879A0, 0x3E7);
	}

	void	Sunrise(MenuEntry *entry)
	{
		if (Controller::IsKeysDown(R + DPadLeft))
			WRITEU16(0x0587964, 0x8001);
	}

	void	Daytime(MenuEntry *entry)
	{
		if (Controller::IsKeysDown(R + DPadUp))
			WRITEU16(0x0587964, 0x680E);
	}

	void	Sunset(MenuEntry *entry)
	{
		if (Controller::IsKeysDown(R + DPadRight))
			WRITEU16(0x0587964, 0xC001);
	}

	void	Night(MenuEntry *entry)
	{
		if (Controller::IsKeysDown(R + DPadDown))
			WRITEU16(0x0587964, 0x0000);
	}

	void	UnlockLargeMb(MenuEntry *entry)
	{
		WRITEU8(0x05879A8, 0x01);
	}

	void	UnlockMagic(MenuEntry *entry)
	{
		WRITEU8(0x05879A6, 0x60);
	}

	void	UnlockHeartpieces(MenuEntry *entry)
	{
		WRITEU8(0x0587A17, 0x30);
	}

	void	UnlockEnhancedDefense(MenuEntry *entry)
	{
		WRITEU8(0x005879A9, 0x1);
	}

	void	UnlockAllSwords(MenuEntry *entry)
	{
		u32 backup = READU8(0x00587A0E);
		
		backup = backup & 0xf0;
		backup = backup | 0x7;
		WRITEU8(0x00587A0E, backup);
	}

	void	TriggerSwords(int x)
	{
		u32 backup = READU8(0x00587A0E);
		
		if(x == 0)
			backup = (backup & 0xf0) | ((backup & 0xf) ^ 0b111);
		if(x == 1)
			backup = (backup & 0xf0) | ((backup & 0xf) ^ 0b1);
		if(x == 2)
			backup = (backup & 0xf0) | ((backup & 0xf) ^ 0b10);
		if(x == 3)
			backup = (backup & 0xf0) | ((backup & 0xf) ^ 0b100);
		WRITEU8(0x00587A0E, backup);
	}

	void	TriggerShields(int x)
	{
		u32 backup = READU8(0x00587A0E);
		
		if(x == 0)
			backup = (backup & 0x0f) | ((backup & 0xf0) ^ (0b111 << 4));
		if(x == 1)
			backup = (backup & 0x0f) | ((backup & 0xf0) ^ ( 0b1 << 4));
		if(x == 2)
			backup = (backup & 0x0f) | ((backup & 0xf0) ^ ( 0b10 << 4));
		if(x == 3)
			backup = (backup & 0x0f) | ((backup & 0xf0) ^ ( 0b100 << 4));
		WRITEU8(0x00587A0E, backup);
	}

	void	UnlockAllShields(MenuEntry *entry)
	{
		u32 backup = READU8(0x00587A0E);
		
		backup = backup & 0xf;
		backup = backup | 0x70;
		WRITEU8(0x00587A0E, backup);
	}

	void	UnlockAllSuits(MenuEntry *entry)
	{
		WRITEU8(0x0587A0F, 0x7);
	}

	void	TriggerSuits(int x)
	{
		u32 backup = READU8(0x00587A0F);
		
		if(x == 0)
			backup = (backup & 0xf0)| ((backup & 0xf) ^ 0b111);
		if(x == 1)
			backup = (backup & 0xf0)| ((backup & 0xf) ^ 0b1);
		if(x == 2)
			backup = (backup & 0xf0) | ((backup & 0xf) ^ 0b10);
		if(x == 3)
			backup = (backup & 0xf0) | ((backup & 0xf) ^ 0b100);
		WRITEU8(0x00587A0F, backup);
	}

	void	UnlockAllStones(MenuEntry *entry)
	{
		// 0x04 pierre foret
		// 0x08 pierre feu
		// 0x10 pierre eau
		// 0x20 pierre souffrrance	
		// 0x40 gerudo token
		// 
		WRITEU8(0x0587A16, 0x7C);
	}

	void	UnlockAllBest(MenuEntry *entry)
	{
		// 0x587A11 : 
		//				0x40 30x seedbag, 
		//				0x80 40x seed bag, 
		//				0xC0 50x seedbag,
		//				0x02 silver Zora scale, 
		//				0x04 gold Zora scale, 
		//				0x06 Broken Biggoron,
		//
		WRITEU32(0x0587A10, 0x184D8);
	}

	void	UnlockAllMedallions(MenuEntry *entry)
	{
		WRITEU8(0x0587A14, 0x3F);
	}

	void	GiantLink(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);

		if (!pointer)
			return;
		pointer += 0x64;
		WRITEU32(pointer, 0x3CA3D70A);
		WRITEU32(pointer + 4, 0x3CA3D70A);
		WRITEU32(pointer + 8, 0x3CA3D70A);
		return;
	}

	void	NormalLink(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);

		if (!pointer)
			return;
		pointer += 0x64;
		WRITEU32(pointer, 0x3C23D70A);
		WRITEU32(pointer + 4, 0x3C23D70A);
		WRITEU32(pointer + 8, 0x3C23D70A);
		return;
	}

	void 	MiniLink(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);

		if (!pointer)
			return;
		pointer += 0x64;
		WRITEU32(pointer, 0x3B23D70A);
		WRITEU32(pointer + 4, 0x3B23D70A);
		WRITEU32(pointer + 8, 0x3B23D70A);
		return;
	}

	void	PaperLink(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);

		if (!pointer)
			return;
		pointer += 0x64;
		WRITEU32(pointer, 0x3AD3D70A);
		return;
	}

	void	MoonJump(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);

		if (!pointer)
			return;
		pointer += 0x77;	
		if (Controller::IsKeyDown(A))
			WRITEU16(pointer, 0xCB40);
		return;
	}

	void	MoveFast(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);
		u32 jumpPointer;
		static u32 jump = 0;
		
		if (!pointer)
			return;
		jumpPointer = pointer + 0x77;
		pointer += 0x222C;

		if (Controller::IsKeyDown(Key::CPad) && Controller::IsKeyDown(Key::ZL))
		{
			if (jump < 3)
			{
				WRITEU16(jumpPointer, 0xCB40);
				WRITEU16(jumpPointer, 0xCB40);
				jump++;
			}
			else
				WRITEU32(pointer, 0x41A00000);
		}
		else
			jump = 0;
		return;
	}

	void	CollectHeart(MenuEntry *entry)
	{
		WRITEU16(0x08720A84, 0x000000000);
	}

	void	OpenChest(MenuEntry *entry)
	{
		WRITEU32(0x08720A78, 0x000000000);
	}

	void	SpinAttack(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);
		
		if (!pointer)
			return;
		pointer += 0x2252;
		WRITEU16(pointer, 0x3F80);
	}

	void	SwordGlitch(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);

		if (!pointer)
			return;
		pointer += 0x2237;
		WRITEU8(pointer, 0x01);
	}

	void	StickFire(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);

		if (!pointer)
			return;
		pointer += 0x2258;
		WRITEU8(pointer, 0xFF);
	}

	void	EponaMaxCarrots(MenuEntry *entry)
	{
		u32	pointer = READU32(basePointer);
		
		if (!pointer)
			return;
		pointer += 0x134;
		WRITEU8(pointer, 0x5);
	}


	void	UnlockKokiriSword(MenuEntry *entry)
	{
		if (entry->IsActivated())
		TriggerSwords(1);
		entry->Disable();
	}

	void	UnlockExcaliburSword(MenuEntry *entry)
	{
		if (entry->IsActivated())
		TriggerSwords(2);
		entry->Disable();
	}

	void 	UnlockBiggoronSword(MenuEntry *entry)
	{
		if (entry->IsActivated())
		TriggerSwords(3);
		entry->Disable();
	}

	void	UnlockWoodShield(MenuEntry *entry)
	{
		if (entry->IsActivated())
		TriggerShields(1);
		entry->Disable();
	}

	void	UnlockHyruleShield(MenuEntry *entry)
	{
		if (entry->IsActivated())
		TriggerShields(2);
		entry->Disable();
	}

	void	UnlockMirrorShield(MenuEntry *entry)
	{
		if (entry->IsActivated())
		TriggerShields(3);
		entry->Disable();
	}

	void	UnlockKokiriSuits(MenuEntry *entry)
	{
		if (entry->IsActivated())
		TriggerSuits(1);
		entry->Disable();
	}

	void	UnlockZoraSuits(MenuEntry *entry)
	{
		if (entry->IsActivated())
		TriggerSuits(3);
		entry->Disable();
	}

	void	UnlockGoronSuits(MenuEntry *entry)
	{
		if (entry->IsActivated())
		TriggerSuits(2);
		entry->Disable();
	}


	void	AlwaysAdultLinkVoice(MenuEntry *entry)
	{
		WRITEU32(0x098F671C, 0x0053A2F0);
	}

	void	AlwaysChildLinkVoice(MenuEntry *entry)
	{
		WRITEU32(0x098F671C, 0x0053A424);
	}

	void	InfiniteSmallKeysAllDungeons(MenuEntry *entry)
	{
		for (int i = 0; i < 0x10; i++)
		{
			WRITEU32(0x00587A2C + i, 0x09090909);
		}
	}

	void	HaveMapCompassAndBossKeyAllDungeon(MenuEntry *entry)
	{
		for (int i = 0; i < 0x00000006; i++)
		{
			WRITEU32(0x00587A18 + i, 0x07070707);
		}
	}

	void	EponaMoonJump(MenuEntry *entry)
	{
		u32	offset;

		offset = READU32(basePointer);
		if (!offset)
			return;
		offset += 0x134;
		if (Controller::IsKeysDown(Key::ZL + Key::X))
		{
			if ( 0x00000000 != READU32(offset))
			{
				offset = READU32(offset);
				WRITEU16(0x00000066 + offset, 0x4222);
			}
		}
	}

	void	EponaInfiniteCarrotsAllAreas(MenuEntry *entry)
	{
		u32	offset;

		offset = READU32(basePointer);
		if (!offset)
			return;
		offset += 0x134;
		if ( 0 != READU32(offset))
		{
			offset = READU32(offset);
			WRITEU8(0xE9C + offset, 5);
		}
	}

	void	GiantsKnifeNeverBreaks(MenuEntry *entry)
	{
		WRITEU8(0x005879A2, 0xFF);
	}

	void	AlwaysRaining(MenuEntry *entry)
	{
		WRITEU8(0x08721AAF, 0xFF);
	}

	void	OpenAnyChestInTheGameAsManyTimes(MenuEntry *entry)
	{
		WRITEU32(0x08720A78, 0x00000000);
	}

	void	CollectHeartPiecesInOverworldAsMany(MenuEntry *entry)
	{
		WRITEU16(0x08720A84, 0);
	}

	void	AlwaysHaveNayrusLoveActivated(MenuEntry *entry)
	{
		WRITEU16(0x00588EAA, 0x0000FFFF);
	}

	void	NeverTakeDamageFromFalling(MenuEntry *entry)
	{
		WRITEU32(0x098F7290, 0x00000000);
	}
}