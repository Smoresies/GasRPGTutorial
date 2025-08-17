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
### Ability Actor Info
The Ability System Component knows that it might be owned be owned by an actor, pawned, or other type of entity 
(such as PlayerState). Therefore it has 2 variables to designate. One that stores Owner Actor (InOwnerActor) and one 
that stores Avatar Actor (InAvatarActor) which are designated using InitAbilityActorInfo(). 

Owner Actor will hold whatever has constructed it/is designated to hold the Ability System Info (For AuraPlayer this is 
PlayerState, but for Enemies this is the Enemy). The Avatar Actor will be whichever Pawn/Character that is designated by the Owner Actor. In the example of 
Aura tutorial the Avatar Actor will be our Player (the Aura we move around on screen) while the Owner Actor will be the 
PlayerState

When do we call set this information (call the InitAbilityActorInfo) in a multiplayer game?

Main Rule to follow:
Must be done AFTER Possession (the Controller has been set for the Pawn)

**For a Player-Controlled Characters:** 

When ASC lives on Pawn (not done in this project) then a good place to call 
     would be the PossessedBy function on the Pawn (server-side) and AcknowledgePossession (client-side). 

 When the ASC lives on Player State we use PossessedBy (server-side) and 
     OnRep_PlayerState (client-side). The change here is because we need to make sure the 
     controller is set AND PlayerState is valid. 
 
OnRep_PlayerState is Safe (so it will always be done by then) 
     and is a Rep-notify (A function called as a result of something being replicated). In this case 
     the PlayerState will have been replicated - which will trigger the notify, and this flags that we are safe to 
     initialize Ability Actor Info.

**For AI-Controlled Characters:**

When ASC lives on Pawn we only need to call InitAbilityActorInfo() in BeginPlay. We know that AI is going to have 
valid ASC and Controller, so we can call it safely with no worries.


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

### Special notes for Mixed Replication Mode:
For Mixed Replication Mode: The OwnerActor's Owner (when doing InitAbilityActorInfo for the AbilityComponentSystem) 
must be the Controller. For Pawns this is set automatically in PossessedBy().

The PlayerStates's Owner is automatically set to the Controller.

Therefore, if your OwnerActor is not the PlayerState, and you use Mixed Replication Mode, you must call SetOwner() 
on the OwnerActor to set its owner to the Controller. This appears to mostly matter if you are creating your own 
storage system for your ACS? If your OwnerActor is some class that does not automatically get its owner set to the 
Controller. 

You might have an AbilitySystemComponent on some Actor that does not have a Controller... That's fine? Don't worry 
about it? But in Mixed Replication the OwnerActor MUST be owned by the Controller

[INSERT REASON WHY WE USE MIXED HERE?!?!]

## Set Net Update Frequency
This global engine function SetNetUpdateFrequency takes a float to determine 
how often the server will try to update clients.
As changes occur on server for player state, server will send updates out. This is
normally quite low, like half a second. 
Apparently 100 is high and means 100 updates per second