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
  "onEffectAdded",
  "onEffectUpdated",
  "onEffectRemoved",
  "onStartDestroyBlock",
  "onDestroyBlock",
  "onPlaceBlock",
  "afterPlaceBlock",
  "onOpenContainer",
  "onCloseContainer",
  "onInventoryChange",
  "onPlayerPullFishingHook",
  "onPlayerInteractEntity",
  "onChangeSprinting",
  "onSetArmor",
  "onUseRespawnAnchor",
  "onOpenContainerScreen",
  "onExperienceAdd",
  "onBedEnter",
  "onOpenInventory",
  /* Entity Events */
  "onMobDie",
  "onMobHurt",
  "onEntityExplode",
  "onProjectileHitEntity",
  "onWitherBossDestroy",
  "onRide",
  "onSpawnProjectile",
  "onProjectileCreated",
  "onEntityTransformation",
  "onMobTrySpawn",
  "onMobSpawned",
  "onPortalTrySpawnPigZombie",
  "onNpcCmd",
  "onEndermanTakeBlock",
  /* Block Events */
  "onBlockInteracted",
  "onBlockChanged",
  "onBlockExplode",
  "onRespawnAnchorExplode",
  "onPortalTrySpawn",
  "onBlockExploded",
  "onFireSpread",
  "onCmdBlockExecute",
  "onContainerChange",
  "onDispenseItem",
  "onProjectileHitBlock",
  "onRedStoneUpdate",
  "onHopperSearchItem",
  "onHopperPushOut",
  "onPistonTryPush",
  "onPistonPush",
  "onFarmLandDecay",
  "onUseFrameBlock",
  "onLiquidFlow",
  "onStepOnPressurePlate",
  "onChangeArmorStand",
  /* Other Events */
  "onScoreChanged",
  "onTick",
  "onServerStarted",
  "onConsoleCmd",
  /* Economic Events */
  "onMoneyAdd",
  "onMoneyReduce",
  "onMoneyTrans",
  "onMoneySet",
  "beforeMoneyAdd",
  "beforeMoneyReduce",
  "beforeMoneyTrans",
  "beforeMoneySet",
  /* Outdated Events */
  "onAttack",
  "onExplode",
  "onBedExplode",
  "onMobSpawn",
];

export const triggeredEvents = new Set();

export function RegisterEvents() {
  events.forEach((event) => {
    mc.listen(event, (...args) => {
      const logEvent = () => {
        var out = "";
        args.forEach((item) => {
          out += "[" + item + "] ";
        });
        logger.info(`${event}: ${out}`);
      };
      if (!triggeredEvents.has(event)) {
        triggeredEvents.add(event);
        logEvent();
        logger.info(`${triggeredEvents.size}/${events.length} events called`);
        return;
      }
      if (event != "onTick") {
        logEvent();
      }
    });
  });
}
