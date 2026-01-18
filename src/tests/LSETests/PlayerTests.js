export function TestPlayer(output, players) {
    for (let player of players) {
        GetFromViewVector(output, player)
        PrintPlayerAttributes(output, player)
    }
}

function GetFromViewVector(output, player) {
    let en = player.getEntityFromViewVector(5.25);
    if (en) {
        output.success(`Entity looking at: ${en.name}`);
        logBlock(output, player.getBlockFromViewVector(true, false, player.distanceTo(en), false));
    } else {
        output.success(`No entity looking at found`);
        logBlock(output, player.getBlockFromViewVector(true, false, 5.25, false));
    }

    function logBlock(output, bl) {
        if (bl) {
            output.success(`Block looking at: ${bl.name} ${bl.pos}`);
        } else {
            output.success(`No Block looking at found`);
        }
    }
}

function PrintPlayerAttributes(output, player) {
    output.success(`Name: ${player.name}`);
    output.success(`Position: ${player.pos}`);
    output.success(`Feet Position: ${player.feetPos}`);
    output.success(`Block Position: ${player.blockPos}`);
    output.success(`Last Death Position: ${player.lastDeathPos}`);
    output.success(`Real Name: ${player.realName}`);
    output.success(`XUID: ${player.xuid}`);
    output.success(`UUID: ${player.uuid}`);
    output.success(`Permission Level: ${player.permLevel}`);
    output.success(`Game Mode: ${player.gameMode}`);
    output.success(`Can Sleep: ${player.canSleep}`);
    output.success(`Can Fly: ${player.canFly}`);
    output.success(`Can Be Seen On Map: ${player.canBeSeenOnMap}`);
    output.success(`Can Freeze: ${player.canFreeze}`);
    output.success(`Can See Daylight: ${player.canSeeDaylight}`);
    output.success(`Can Show Name Tag: ${player.canShowNameTag}`);
    output.success(`Can Start Sleep In Bed: ${player.canStartSleepInBed}`);
    output.success(`Can Pickup Items: ${player.canPickupItems}`);
    output.success(`Max Health: ${player.maxHealth}`);
    output.success(`Health: ${player.health}`);
    output.success(`In Air: ${player.inAir}`);
    output.success(`In Water: ${player.inWater}`);
    output.success(`In Lava: ${player.inLava}`);
    output.success(`In Rain: ${player.inRain}`);
    output.success(`In Snow: ${player.inSnow}`);
    output.success(`In Wall: ${player.inWall}`);
    output.success(`In Water Or Rain: ${player.inWaterOrRain}`);
    output.success(`In World: ${player.inWorld}`);
    output.success(`In Clouds: ${player.inClouds}`);
    output.success(`Speed: ${player.speed}`);
    output.success(`Direction: ${player.direction}`);
    output.success(`Unique ID: ${player.uniqueId}`);
    output.success(`Language Code: ${player.langCode}`);
    output.success(`Is Loading: ${player.isLoading}`);
    output.success(`Is Invisible: ${player.isInvisible}`);
    output.success(`Is Inside Portal: ${player.isInsidePortal}`);
    output.success(`Is Hurt: ${player.isHurt}`);
    output.success(`Is Trusting: ${player.isTrusting}`);
    output.success(`Is Touching Damage Block: ${player.isTouchingDamageBlock}`);
    output.success(`Is Hungry: ${player.isHungry}`);
    output.success(`Is On Fire: ${player.isOnFire}`);
    output.success(`Is On Ground: ${player.isOnGround}`);
    output.success(`Is On Hot Block: ${player.isOnHotBlock}`);
    output.success(`Is Trading: ${player.isTrading}`);
    output.success(`Is Adventure: ${player.isAdventure}`);
    output.success(`Is Gliding: ${player.isGliding}`);
    output.success(`Is Survival: ${player.isSurvival}`);
    output.success(`Is Spectator: ${player.isSpectator}`);
    output.success(`Is Riding: ${player.isRiding}`);
    output.success(`Is Dancing: ${player.isDancing}`);
    output.success(`Is Creative: ${player.isCreative}`);
    output.success(`Is Flying: ${player.isFlying}`);
    output.success(`Is Sleeping: ${player.isSleeping}`);
    output.success(`Is Moving: ${player.isMoving}`);
    output.success(`Is Sneaking: ${player.isSneaking}`);
}
