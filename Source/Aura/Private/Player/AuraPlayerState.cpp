// Copyright Smoresies


#include "Player/AuraPlayerState.h"

AAuraPlayerState::AAuraPlayerState()
{
	/**
	 *  How often the server will try to update clients.
	 *  As changes occur on server for player state, server will send updates out
	 *  Normally quite low, like half a second. If we had the attribute set on state, we would make higher
	 *  Apparently 100 is high? 100 updates per second
	**/
	SetNetUpdateFrequency(100.f);
}
