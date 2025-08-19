<!-- TOC -->
* [General Unreal](#general-unreal)
  * [UFUNCTION()](#ufunction)
  * [UPROPERTY()](#uproperty)
  * [Debug](#debug)
  * [UI](#ui)
    * [Widgets](#widgets)
* [Gas](#gas)
  * [PlayerState](#playerstate)
  * [UAbilitySystemComponent](#uabilitysystemcomponent)
    * [PlayerState vs Pawn-Owned](#playerstate-vs-pawn-owned)
    * [Ability Actor Info](#ability-actor-info)
  * [UAttributeSet](#uattributeset)
    * [What are Attributes:](#what-are-attributes)
  * [Creating Attributes](#creating-attributes)
  * [Attribute Getters/Setters](#attribute-getterssetters)
  * [Effect Actor](#effect-actor)
* [Multiplayer](#multiplayer)
  * [Replication](#replication)
  * [Set Replicated Mode](#set-replicated-mode)
    * [Special notes for Mixed Replication Mode:](#special-notes-for-mixed-replication-mode)
  * [Set Net Update Frequency](#set-net-update-frequency)
  * [Prediction](#prediction)
    * [Client-Side Change Example WITHOUT Prediction](#client-side-change-example-without-prediction)
    * [Client-Side Change WITH Prediction](#client-side-change-with-prediction)
<!-- TOC -->

# General Unreal
## UFUNCTION()
A list of the UFUNCTION tags and what they do:

* BlueprintImplementableEvent
  * Allows us to implement/define this function in Blueprint, when creating a BP class of this type
* BlueprintCallable
  * Callable from Blueprint, will run whatever code we've implemented in C++

## UPROPERTY()
A list of the UPROPERTY tags and what they do:

* BlueprintReadOnly
  * Can be accessed in Blueprint but is treated as const (cannot be changed)
* Category
  * ??? Seems to just be sorting so far?
* 

## Debug

You can use up arrow on keyboard to reference previous commands

* showdebug abilitysystem - Commands for debugging AbilitySystem
    * Can use Page Up and Page Down to change who the debug is shown for.
    * Shows Attributes for Avatar [Class] for owner [Class]. If you're using a BP it should show the BP_Character here
    * Shows name of computer (name of whatever computer is the client to it?), location/rotation, instigator,
      controller, and more
    * Shows Owned Tags
    * Shows Attributes
## UI
Primarily we use the MVC (Model, View, Controller) implementation for UI in Unreal. The UI itself should be the View,
influenced by the Model which is done by the Controller. The view itself will only need to worry about how the UI 
needs to look. Any functionality (button pressing for example) will be handled by the Controller to then signal the 
Model to make any necessary changes.

This should be developed in such a way that the View (Widgets) can be changed out without the Controller caring, and 
the Controller can be swapped out without the Model (backend) caring.

### Widgets
Widgets inheirit from UUserWidget and are used primarily for our front-end UI information. These will directly 
effect how our HUD looks.

Widgets should know what their controller is (in MVC Architecture) but not the other way around.

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
This is automatically registered to the ACS it is constructed alongside. Additionally, you can have multiple 
AttributeSets registered into an ACS. For example: if you wanted to have attributes onto different types (Primary, 
Secondary, Tertiary) you may... BUT These must be separate class-types (you cannot have more than some attributeset 
of a given class).

Attribute sets have neglible memory and thus you can have one for each possible class. In this project we are using 
only a single AttributeSet for all Characters in the game and can grab only the needed attributes from the set for 
each Character

### What are Attributes:
Attributes are numberical quantities associated with a given entity in the game (such as a character), all 
attributes are floats, exist with FGameplayAttributeData, and are stored on the AttributeSet. The AttributeSet keeps 
Attributes under close supervision. We can know when an attribute changes and respond with any needed functionality.

Attributes _can_ be set directly in code but the preferred methodology is by using Gameplay Effects (which can 
change attributes in a number of different ways). Aside from the built-in capabilities of Gameplay Effects, another 
reason to use them is that Gameplay Effects allow us to predict changes to attributes (Apply Prediction).

Attributes consist of 2 actual values: A Base Value and a Current Value.

The Base Value is the Permanent value of the attribute.

The Current Value is the base value PLUS any temporary modifications from Gameplay Effects... Think of Buffs/Debuffs,
you may have an effect that adds or subtracts a value for a period of time... But when that tiem is up, it goes back 
to the Base Value.

The Max Value is separate from the Base Value... If the Max Attribute can CHANGE you should have this have this as 
it's own Attribute (Health and Max Health would be two separate Attributes)

## Creating Attributes
When creating an Attribute in your AttributeSet class there are 4 steps to follow:

1 - Add the member variable. This should be of type FGameplayAttributeData, should have UProperty, and will likely 
need to be replicated (although some may not). Example:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
	FGameplayAttributeData Health;

If your member variable is NOT replicated, then stop here! Otherwise...

2 - Create the replication functions. These can take either zero or one parameters. If they take a parameter then 
that parameter should be the OLD value of the given Attribute AND should be a const reference. Example:

    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

After you've prototyped this function the declaration should looks as such: 

    void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
    {
        GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
    }

This is providing the Rep Notify signal for specifically a Gameplay Attribute. It takes the class type of the 
Attribute, new value, and old value.

3 - Lastly you need to add this into a new overriden function "GetLifetimeReplicatedProps". This declaration is:

    void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
    {
        Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
    }
The DOREPLIFETIME_CONDITION_NOTIFY macro registers the member variable/attribute to be replicated. In this example 
it is replicated without conditions and if the value is set on the server we will always replicate it

There is an additional option for the last parameter "REPNOTIFY_OnChanged" which is an "optimized" possibility, but 
with GAS we ALWAYS want to Rep Notify to potentially reply the effect (even if it didn't change!)

## Attribute Getters/Setters
Attributes SHOULD be adjusted from Gameplay Effects... However, if you cannot for some reason use Gameplay Effects 
you can create Getters and Setters to these!

To use this in your game you can define something like this, and then add game-specific functions as necessary:
    
    #define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
    
    ATTRIBUTE_ACCESSORS(UMyHealthSet, Health)

## Effect Actor
[TBD]

# Multiplayer
## Replication
Replication is the sending of server-side information back to the client-side versions of the game to keep the 
"correct" version of the game flowing at all points in time. In this way the server is seen as the "One True Game" 
in which all client copies _should_ be matching as closely as possible. Replication allows us to update clients 
quickly and often to ensure they're kept as up to date to the Server as possible.

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

## Prediction
The client doesn't need to wait for the server's permission to change a value. The value can change
immediately client-side and the server is informed of the change. The server can roll back changes that are invalid.
This seems to be our only way of Replicating back to the server from the client (albeit in a limited sense).

Prediction makes for a smoother experience in multiplayer - "Something you don't notice when it is there, but do
notice when it's not".

### Client-Side Change Example WITHOUT Prediction
On Client you have an attribute that needs to change. Since the server should be in charge of important gameplay 
changes (otherwise clients would be able to cheat), a request is sent to the server to tell it that the attribute's 
value needs to change. 

The server receives the request, decides if the attribute needs to change (by any number of criteria set by the 
developers), and if that change is deemed valid that change is then sent back to the Client (who can now update that 
change).

This results in a noticable delay due to the time required to travel across the server, the time the client receives 
the information, and the time it takes to implement the change... sometimes up to 100 
milliseconds or more, which can lead to a very bad gameplay experience.

### Client-Side Change WITH Prediction
With prediction in GAS, a Gameplay Effect modifies an Attribute client-side and that change is perceived
on the client instantly.
No lag times!

Then that change is sent up to the server who still has the responsibility of validating that change.
If the server decides it's a valid change, then it can inform the other clients of the change.
However, if the server decides that the change is not valid (for example, a client hacks the game and
tries to do an ungodly amount of damage) then the server can reject that change and roll
back the changes, setting the client's value back to the correct one. So the server still has authority, but our 
client doesn't have to have a delay.
