-- Routine for NPC "Robert" in the meadows

velocity = 30

loadRoutine = function(R, W)

    if (not W:isQuestComplete("teleport_robert")) then
        R:setDisposed()
        return
    end

	
	R:setTilePosition(60,35.5)
    R:setFacingLeft()
    
    if (not W:isConditionFulfilled("npc_robert2", "talked")) then
        R:setTalkingActive(true)	
    end
    R:setLooped(false)

end	