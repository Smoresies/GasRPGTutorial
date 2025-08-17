<!-- TOC -->
* [Gas](#gas)
  * [PlayerState](#playerstate)
  * [UAbilitySystemComponent](#uabilitysystemcomponent)
    * [PlayerState vs Pawn-Owned](#playerstate-vs-pawn-owned)
  * [UAttributeSet](#uattributeset)
* [Multiplayer](#multiplayer)
  * [Set Replicated Mode](#set-replicated-mode)
  * [Set Net Update Frequency](#set-net-update-frequency)
<!-- TOC -->

# Gas

## PlayerState
Player States (APlayerState) are created for every player on a server (or in a standalone game).
These are replicated to all clients, and contain network game relevant information about the player - such as 
player name, score, etc.

Another definition: Container for variables to replicate between client/server for a specific player. Data container that doesn’t run much logic of his own. Since PlayerController is not available on all clients and Pawns are often destroyed as players die, the PlayerState is a good place to store data that must be replicated or persist between deaths.

## UAbilitySystemComponent
### PlayerState vs Pawn-Owned
As player pawns might be destroyed upon a player's death, the PlayerState allows us to hold the specific information
that each player might need upon respawning... In this case, the UAbilitySystemComponent (and UAttributeSet) can be
owned by the PlayerState in order to prevent it from being lost on death.

In contrast, an enemy will only require their UAbilitySystemComponent/UAttributeSet while they are actively alive,
and therefore they can store the information directly on their pawn (as well as freely lose it upon death). There 
could be an argument to save this on an enemy-specific object pool, but it wouldn't make sense to replicate that to 
each client (and thus shouldn't be a PlayerState).

## UAttributeSet

# Multiplayer
## Set Replicated Mode
The function SetReplicationMode (held in AuraAbilitySystemComponent) takes an Enum of EGameplayEffectReplicationMode.
 
This has Three possible Modes:

		Full - Gameplay effects replicated to ALL CLIENTS.
			   Use for Single Player
		Mixed - Gameplay Effects replicated to owning client only. Gameplay Cues and Gameplay Tags replicated to all clients
			   Multiplayer (Player-Controlled) 
		Minimal - Gameplay Effects are NOT Replicated. Gameplay Cues and Gameplay Tags replicated to all clients
		   Multiplayer (AI-Controlled)
This is used to define which information will be sent to each client during a 
Gameplay Effect. For the most part you _never_ want to use Full, even in single-player
(primarily since Single Player only has a single client to begin with) and will
either end up using Mixed or Minimal. The best rule of thumb is that for anything AI-Controlled
it will use Minimal (Pets, NPCs, Enemies, etc.) while anything Player-Controlled
will use Mixed. 

[INSERT REASON WHY WE USE MIXED HERE?!?!]

## Set Net Update Frequency
This global engine function SetNetUpdateFrequency takes a float to determine 
how often the server will try to update clients.
As changes occur on server for player state, server will send updates out. This is
normally quite low, like half a second. 
Apparently 100 is high and means 100 updates per second