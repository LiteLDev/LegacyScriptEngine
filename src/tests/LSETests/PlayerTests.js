export function TestPlayer(players) {
    for (let player of players) {
        GetFromViewVector(player)
        PrintPlayerAttributes(player)
    }
}

function GetFromViewVector(player) {
    let en = player.getEntityFromViewVector(5.25);
    if (en) {
        logger.info(`Entity looking at: ${en.name}`);
        logBlock(player.getBlockFromViewVector(true, false, player.distanceTo(en), false));
    } else {
        logger.info(`No entity looking at found`);
        logBlock(player.getBlockFromViewVector(true, false, 5.25, false));
    }

    function logBlock(bl) {
        if (bl) {
            logger.info(`Block looking at: ${bl.name} ${bl.pos}`);
        } else {
            logger.info(`No Block looking at found`);
        }
    }
}

function PrintPlayerAttributes(player) {
    logger.info(`Name: ${player.name}`);
    logger.info(`Position: ${player.pos}`);
    logger.info(`Feet Position: ${player.feetPos}`);
    logger.info(`Block Position: ${player.blockPos}`);
    logger.info(`Last Death Position: ${player.lastDeathPos}`);
    logger.info(`Real Name: ${player.realName}`);
    logger.info(`XUID: ${player.xuid}`);
    logger.info(`UUID: ${player.uuid}`);
    logger.info(`Permission Level: ${player.permLevel}`);
    logger.info(`Game Mode: ${player.gameMode}`);
    logger.info(`Can Sleep: ${player.canSleep}`);
    logger.info(`Can Fly: ${player.canFly}`);
    logger.info(`Can Be Seen On Map: ${player.canBeSeenOnMap}`);
    logger.info(`Can Freeze: ${player.canFreeze}`);
    logger.info(`Can See Daylight: ${player.canSeeDaylight}`);
    logger.info(`Can Show Name Tag: ${player.canShowNameTag}`);
    logger.info(`Can Start Sleep In Bed: ${player.canStartSleepInBed}`);
    logger.info(`Can Pickup Items: ${player.canPickupItems}`);
    logger.info(`Max Health: ${player.maxHealth}`);
    logger.info(`Health: ${player.health}`);
    logger.info(`In Air: ${player.inAir}`);
    logger.info(`In Water: ${player.inWater}`);
    logger.info(`In Lava: ${player.inLava}`);
    logger.info(`In Rain: ${player.inRain}`);
    logger.info(`In Snow: ${player.inSnow}`);
    logger.info(`In Wall: ${player.inWall}`);
    logger.info(`In Water Or Rain: ${player.inWaterOrRain}`);
    logger.info(`In World: ${player.inWorld}`);
    logger.info(`In Clouds: ${player.inClouds}`);
    logger.info(`Speed: ${player.speed}`);
    logger.info(`Direction: ${player.direction}`);
    logger.info(`Unique ID: ${player.uniqueId}`);
    logger.info(`Language Code: ${player.langCode}`);
    logger.info(`Is Loading: ${player.isLoading}`);
    logger.info(`Is Invisible: ${player.isInvisible}`);
    logger.info(`Is Inside Portal: ${player.isInsidePortal}`);
    logger.info(`Is Hurt: ${player.isHurt}`);
    logger.info(`Is Trusting: ${player.isTrusting}`);
    logger.info(`Is Touching Damage Block: ${player.isTouchingDamageBlock}`);
    logger.info(`Is Hungry: ${player.isHungry}`);
    logger.info(`Is On Fire: ${player.isOnFire}`);
    logger.info(`Is On Ground: ${player.isOnGround}`);
    logger.info(`Is On Hot Block: ${player.isOnHotBlock}`);
    logger.info(`Is Trading: ${player.isTrading}`);
    logger.info(`Is Adventure: ${player.isAdventure}`);
    logger.info(`Is Gliding: ${player.isGliding}`);
    logger.info(`Is Survival: ${player.isSurvival}`);
    logger.info(`Is Spectator: ${player.isSpectator}`);
    logger.info(`Is Riding: ${player.isRiding}`);
    logger.info(`Is Dancing: ${player.isDancing}`);
    logger.info(`Is Creative: ${player.isCreative}`);
    logger.info(`Is Flying: ${player.isFlying}`);
    logger.info(`Is Sleeping: ${player.isSleeping}`);
    logger.info(`Is Moving: ${player.isMoving}`);
    logger.info(`Is Sneaking: ${player.isSneaking}`);
}