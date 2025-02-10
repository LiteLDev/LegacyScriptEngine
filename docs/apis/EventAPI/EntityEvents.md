# 🎈 Entity Related Events

#### `"onMobDie"` - Mob/Player Death Event

- Listener function prototype
  `function(mob,source,cause)`
- Parameters:
    - mob : `Entity`  
      Dead entity object.
    - source : `Entity`  
      The entity object that dealt the last damage (may be `Null`).
    - cause : `Integer`   
      Cause of injury
- Intercept event: cannot be intercepted.

Note that when the player dies, in addition to triggering `onPlayerDie` event, this event will also be triggered once.

#### `"onMobHurt"` - Mob/Player Hurt Event

- Listener function prototype
  `function(mob,source,damage,cause)`
- Parameters:
    - mob : `Entity`  
      The damaged entity.
    - source : `Entity`  
      The entity that dealt the damage (may be `Null`).
    - damage : `Integer`  
      The amount of damage dealt.
    - cause : `ActorDamageCause`   
      Cause of injury

- Intercept events: function returns `false`

Here are values of `ActorDamageCause`:

| ActorDamageCause | Enum Value |
|------------------|------------|
| None             | -1         |
| Override         | 0          |
| Contact          | 1          |
| EntityAttack     | 2          |
| Projectile       | 3          |
| Suffocation      | 4          |
| Fall             | 5          |
| Fire             | 6          |
| FireTick         | 7          |
| Lava             | 8          |
| Drowning         | 9          |
| BlockExplosion   | 10         |
| EntityExplosion  | 11         |
| Void             | 12         |
| Suicide          | 13         |
| Magic            | 14         |
| Wither           | 15         |
| Starve           | 16         |
| Anvil            | 17         |
| Thorns           | 18         |
| FallingBlock     | 19         |
| Piston           | 20         |
| FlyIntoWall      | 21         |
| Magma            | 22         |
| Fireworks        | 23         |
| Lightning        | 24         |
| Charging         | 25         |
| Temperature      | 26         |
| Freezing         | 27         |
| Stalactite       | 28         |
| Stalagmite       | 29         |
| RamAttack        | 30         |
| SonicBoom        | 31         |
| Campfire         | 32         |
| SoulCampfire     | 33         |
| MaceSmash        | 34         |
| All              | 35         |

#### `"onEntityExplode"` - Entity Explosion Event

- Listener function prototype
  `function(source,pos,radius,maxResistance,isDestroy,isFire)`
- Parameters:
    - source : `Entity`  
      The entity object that caused the explosion.
    - pos : `FloatPos`  
      The coordinates of the explosion.
    - radius : `Float`    
      Blast radius.
    - maxResistance : `Float`  
      The maximum resistance of blocks that will break.
    - isDestroy : `Boolean`  
      Does the explosion destroy blocks.
    - isFire : `Boolean`  
      Does the explosion produce flames.

- Intercept events: function returns `false`

#### `"onTryMobSpawn"` - Mob try Naturally Spawn Event

- Listener function prototype
  `function(typeName,pos)`
- Parameters:
    - typeName : `string`  
      Entity Name
    - pos : `FloatPos`  
      The coordinates of the spawn.

- Intercept events: function returns `false`

#### `"onMobSpawned"` - Mob Naturally Spawn Finished Event

- Listener function prototype
  `function(entity,pos)`
- Parameters:
    - entity: `Entity`  
      The entity that spawned.
    - pos : `FloatPos`  
      The coordinates of the spawn.

- Intercept events: cannot be intercepted.

You can use entity.despawn() or entity.remove() to intercept this event.

#### `"onProjectileHitEntity"` - Entity Hit by Projectile Event

- Listener function prototype
  `function(entity,source)`
- Parameters:
    - entity: `Entity`  
      The entity that was hit with a projectile.
    - source : `Entity`  
      The projecticle entity (like arrows).
- Intercept event: cannot be intercepted.

#### `"onWitherBossDestroy"` - Block Broken by Wither Event

- Listener function prototype
  `function(witherBoss,AAbb,aaBB)`
- Parameters:
    - witherBoss: `Entity`  
      The Wither entity object.
    - AAbb: `IntPos`
      The area that the wither will destroy (box), the A coordinate of the diagonal point.
    - aaBB: `IntPos`
      The area that the wither will destroy (box), the B coordinate of the diagonal point.

- Intercept events: function returns `false`

Note that this event does not include wither explosion damage.

#### `"onRide"` - Mob Ride Event

- Listener function prototype
  `function(entity1,entity2)`
- Parameters:
    - entity1 : `Entity`  
      The entity that is riding the other entity.
    - entity2 : `Entity`  
      The entity that is being ridden.
- Intercept events: function returns `false`

Note: Riding includes minecart, boat, horse, pig, etc.

#### `"onStepOnPressurePlate"` - Pressure Plate Step Event

- Listener function prototype
  `function(entity,pressurePlate)`
- Parameters:
    - entity : `Entity`  
      The entity that stepped on the plate.
    - pressurePlate : `Block`  
      The pressed pressure plate block.
- Intercept events: function returns `false`

Note: When a creature steps on a pressure plate, this event will be triggered repeatedly.

#### `"onSpawnProjectile"` - Projectile Spawn Event

- Listener function prototype
  `function(shooter,type)`
- Parameters:
    - shooter : `Entity`  
      The entity that fired the projectile.
    - type : `String`  
      Projectile Standard Type Name.

- Intercept events: function returns  `false`

Note: Projectiles known to be intercepted are eggs, ender pearls, snowballs, tridents, arrows, and fishing rods (fish
hooks).

#### `"onProjectileCreated"` - Projectile Created Event

- Listener function prototype
  `function(shooter,entity)`
- Parameters:
    - shooter : `Entity`  
      The entity that created the projectile.
    - entity : `Entity`  
      The projectile entity object being created.

- Intercept event: cannot be intercepted.

#### `"onNpcCmd"` - NPC Command Execution Event

!!! warning
    This event is only available in 0.9.6 and later versions.

- Listener function prototype
  `function(npc,pl,cmd)`
- Parameters:
    - npc : `Entity`  
      The NPC entity that executed the command.
    - pl : `Player`  
      The player that triggered the execution of the NPC command.
    - cmd : `String`  
      The command being executed by NPCs. If there are multiple commands, they are separated by `;`.
- Intercept events: function returns `false`

#### `"onChangeArmorStand"` - Armor Stand Change Event

- Listener function prototype
  `function(as,pl,slot)`

- Parameters:

    - as: `Entity`  
      Manipulated Armor Stand entity object.
    - pl : `Player`  
      The player that manipulated the armor stand.
    - slot : `Number`
      Equipment slot number.

- Intercept events: function returns `false`

#### `"onEntityTransformation"` - Entity Transformation Event

- Listener function prototype
  `function(uniqueId,entity)`
- Parameters:

    - uniqueId: `String`
      Unique identifer of the pre-transformed entity.
    - entity : `Entity`
      The transformed entity.
- Intercept event: cannot be intercepted.

Note: This event is triggered when the `TransformationComponent` of the entity in `Addons` is activated, and is mostly
used for the interaction between the engine and the Addon. Only `UniqueId` is provided since the entity pointer before
the transition is destroyed quickly.
