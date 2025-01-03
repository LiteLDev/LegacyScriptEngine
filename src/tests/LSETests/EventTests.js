export const events = [
    "onPreJoin",
    "onJoin",
    "onLeft",
    "onRespawn",
    "onPlayerDie",
    "onPlayerCmd",
    "onChat",
    "onChangeDim",
    "onJump",
    "onSneak",
    "onPlayerSwing",
    "onAttackEntity",
    "onAttackBlock",
    "onUseItem",
    "onUseItemOn",
    "onUseBucketPlace",
    "onUseBucketTake",
    "onTakeItem",
    "onDropItem",
    "onEat",
    "onAte",
    "onConsumeTotem",
    "onStartDestroyBlock",
    "onDestroyBlock",
    "onPlaceBlock",
    "afterPlaceBlock",
    "onOpenContainer",
    "onCloseContainer",
    "onInventoryChange",
    "onPlayerPullFishingHook",
    "onChangeSprinting",
    "onSetArmor",
    "onUseRespawnAnchor",
    "onOpenContainerScreen",
    "onExperienceAdd",
    "onBedEnter",
    "onOpenInventory",
    "onMobDie",
    "onMobHurt",
    "onEntityExplode",
    "onProjectileHitEntity",
    "onWitherBossDestroy",
    "onRide",
    "onStepOnPressurePlate",
    "onSpawnProjectile",
    "onProjectileCreated",
    "onChangeArmorStand",
    "onBlockInteracted",
    "onBlockChanged",
    "onBlockExplode",
    "onRespawnAnchorExplode",
    "onBlockExploded",
    "onFireSpread",
    "onCmdBlockExecute",
    "onContainerChange",
    "onProjectileHitBlock",
    "onRedStoneUpdate",
    "onHopperSearchItem",
    "onHopperPushOut",
    "onPistonTryPush",
    "onPistonPush",
    "onFarmLandDecay",
    "onUseFrameBlock",
    "onLiquidFlow",
    "onScoreChanged",
    "onServerStarted",
    "onConsoleCmd",
    "onMoneyAdd",
    "onMoneyReduce",
    "onMoneyTrans",
    "onMoneySet",
    "beforeMoneyAdd",
    "beforeMoneyReduce",
    "beforeMoneyTrans",
    "beforeMoneySet",
    "onMobTrySpawn",
    "onMobSpawned"
];

export const triggeredEvents = new Set();

export function RegisterEvents() {
    events.forEach(event => {
        mc.listen(event, () => {
            if (triggeredEvents.has(event)) {
                return;
            }
            logger.info(`Event ${event} triggered`);
            triggeredEvents.add(event);
            logger.info(`${triggeredEvents.size}/${events.length} events called`);
        });
    });
}