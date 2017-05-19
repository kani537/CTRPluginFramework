#ifndef CHEATS_H
#define CHEATS_H

#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework
{
    /*
     * Movement
     */
    // A
    void	MoonJump(MenuEntry *entry);
    // ZL + PAD
    void	MoveFast(MenuEntry *entry);
    void	EponaMaxCarrots(MenuEntry *entry);
    void	EponaInfiniteCarrotsAllAreas(MenuEntry *entry);
    // ZL + X
    void	EponaMoonJump(MenuEntry *entry); //x + a

    /*
     * Battle
     */
    // Touchscreen
    void	RefillHeart(MenuEntry *entry);
    // Touchscreen
    void	RefillLargeMagicbar(MenuEntry *entry);
    void	MaxHeart(MenuEntry *entry);
    void	MaxMagic(MenuEntry *entry); // unused
    void	UnlockMagic(MenuEntry *entry);
    void	UnlockLargeMb(MenuEntry *entry);
    void	SpinAttack(MenuEntry *entry);
    void	SwordGlitch(MenuEntry *entry);
    void	StickFire(MenuEntry *entry);
    /*
     * Inventory
     */
    void	UnlockKokiriSword(MenuEntry *entry);
    void	UnlockExcaliburSword(MenuEntry *entry);
    void	UnlockBiggoronSword(MenuEntry *entry);
    void	UnlockAllSwords(MenuEntry *entry);
    void	UnlockWoodShield(MenuEntry *entry);
    void	UnlockHyruleShield(MenuEntry *entry);
    void	UnlockMirrorShield(MenuEntry *entry);
    void	UnlockAllShields(MenuEntry *entry);
    void	UnlockKokiriSuits(MenuEntry *entry);
    void	UnlockZoraSuits(MenuEntry *entry);
    void	UnlockGoronSuits(MenuEntry *entry);
    void	UnlockAllSuits(MenuEntry *entry);
    void	InfiniteArrows(MenuEntry *entry);
    void	InfiniteNuts(MenuEntry *entry);
    void	InfiniteStick(MenuEntry *entry);
    void	InfiniteBomb(MenuEntry *entry);
    void	InfiniteBombchu(MenuEntry *entry);
    void	InfiniteSlingshot(MenuEntry *entry);
    void	Skulltulas(MenuEntry *entry);
    void	MaxRupees(MenuEntry *entry);
    /*
     * Time
     */
    // R + DPadLeft
    void	Sunrise(MenuEntry *entry);
    // R + DPadUp
    void	Daytime(MenuEntry *entry);
    // R + DPadRight
    void	Sunset(MenuEntry *entry);
    // R + DPadLeft
    void	Night(MenuEntry *entry);
    void	AlwaysRaining(MenuEntry *entry);
    /*
     * Quest
     */
    void	UnlockAllBest(MenuEntry *entry);
    void	UnlockAllStones(MenuEntry *entry);
    void	UnlockAllMedallions(MenuEntry *entry);
    void	UnlockEnhancedDefense(MenuEntry *entry);
    void	UnlockHeartpieces(MenuEntry *entry);
    void	InfiniteSmallKeysAllDungeons(MenuEntry *entry);
    void	HaveMapCompassAndBossKeyAllDungeon(MenuEntry *entry);
    /*
     * Misc
     */    
    void	GiantLink(MenuEntry *entry);
    void	NormalLink(MenuEntry *entry);
    void	MiniLink(MenuEntry *entry);
    void	PaperLink(MenuEntry *entry);
    void	AlwaysAdultLinkVoice(MenuEntry *entry);
    void	AlwaysChildLinkVoice(MenuEntry *entry);
    void	OpenChest(MenuEntry *entry);
    void	CollectHeart(MenuEntry *entry);
    void	NeverTakeDamageFromFalling(MenuEntry *entry);
    void	GiantsKnifeNeverBreaks(MenuEntry *entry);
    
    /*
     * Globals functions
     */

    void	TriggerSwords(int x);
    void	TriggerShields(int x);
    void	TriggerSuits(int x);
    
    void	OpenAnyChestInTheGameAsManyTimes(MenuEntry *entry);
    void	CollectHeartPiecesInOverworldAsMany(MenuEntry *entry);
    void	AlwaysHaveNayrusLoveActivated(MenuEntry *entry);

    class Pokemon;
    using VectorPokemon = std::vector<Pokemon>;

    class Pokemon
    {
    public:
        Pokemon(std::string name, u32 id);

        const u32           ID;
        const std::string   Name;
        
        static void     InitList(void);
        static u32      GetList(VectorPokemon &output, std::string pattern = "");
        static Pokemon  &GetPokemon(std::string name);
        static Pokemon  &GetPokemon(u32 id);        
    };

}
#endif
