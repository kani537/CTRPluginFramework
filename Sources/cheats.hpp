#ifndef CHEATS_H
#define CHEATS_H

#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework
{

    void	UnlockKokiriSword(MenuEntry *entry);
    void	UnlockExcaliburSword(MenuEntry *entry);
    void	UnlockBiggoronSword(MenuEntry *entry);
    void	UnlockWoodShield(MenuEntry *entry);
    void	UnlockHyruleShield(MenuEntry *entry);
    void	UnlockMirrorShield(MenuEntry *entry);
    void	UnlockKokiriSuits(MenuEntry *entry);
    void	UnlockZoraSuits(MenuEntry *entry);
    void	UnlockGoronSuits(MenuEntry *entry);

    void	MaxHeart(MenuEntry *entry);
    void	MaxMagic(MenuEntry *entry);
    // Touchscreen
    void	RefillHeart(MenuEntry *entry);
    // Touchscreen
    void	RefillLargeMagicbar(MenuEntry *entry);

    void	InfiniteArrows(MenuEntry *entry);
    void	InfiniteNuts(MenuEntry *entry);
    void	InfiniteStick(MenuEntry *entry);
    void	InfiniteBomb(MenuEntry *entry);
    void	InfiniteBombchu(MenuEntry *entry);
    void	InfiniteSlingshot(MenuEntry *entry);
    void	Skulltulas(MenuEntry *entry);
    void	MaxRupees(MenuEntry *entry);
    
    // R + DPadLeft
    void	Sunrise(MenuEntry *entry);
    // R + DPadUp
    void	Daytime(MenuEntry *entry);
    // R + DPadRight
    void	Sunset(MenuEntry *entry);
    // R + DPadLeft
    void	Night(MenuEntry *entry);

    void	UnlockLargeMb(MenuEntry *entry);
    void	UnlockMagic(MenuEntry *entry);
    void	UnlockHeartpieces(MenuEntry *entry);
    void	UnlockEnhancedDefense(MenuEntry *entry);
    void	UnlockAllSwords(MenuEntry *entry);
    void	UnlockAllShields(MenuEntry *entry);
    void	UnlockAllSuits(MenuEntry *entry);
    void	UnlockAllStones(MenuEntry *entry);
    void	UnlockAllBest(MenuEntry *entry);
    void	UnlockAllMedallions(MenuEntry *entry);
    void	GiantLink(MenuEntry *entry);
    void	NormalLink(MenuEntry *entry);
    void	MiniLink(MenuEntry *entry);
    void	PaperLink(MenuEntry *entry);
    // A
    void	MoonJump(MenuEntry *entry);
    // ZL + PAD
    void	MoveFast(MenuEntry *entry);
    void	MoveFastRev(MenuEntry *entry);
    void	CollectHeart(MenuEntry *entry);
    void	OpenChest(MenuEntry *entry);
    void	SpinAttack(MenuEntry *entry);
    void	SwordGlitch(MenuEntry *entry);
    void	StickFire(MenuEntry *entry);
    void	EponaMaxCarrots(MenuEntry *entry);

    void	TriggerSwords(int x);
    void	TriggerShields(int x);
    void	TriggerSuits(int x);

    void	AlwaysAdultLinkVoice(MenuEntry *entry);
    void	AlwaysAhildLinkVoice(MenuEntry *entry);
    void	InfiniteSmallKeysAllDungeons(MenuEntry *entry);
    void	HaveMapCompassAndBossKeyAllDungeon(MenuEntry *entry);
    // ZL + X
    void	EponaMoonJump(MenuEntry *entry); //x + a
    void	EponaInfiniteCarrotsAllAreas(MenuEntry *entry);
    void	GiantsKnifeNeverBreaks(MenuEntry *entry);
    void	AlwaysRaining(MenuEntry *entry);
    void	OpenAnyChestInTheGameAsManyTimes(MenuEntry *entry);
    void	CollectHeartPiecesInOverworldAsMany(MenuEntry *entry);
    void	AlwaysHaveNayrusLoveActivated(MenuEntry *entry);
    void	NeverTakeDamageFromFalling(MenuEntry *entry);
}
#endif
