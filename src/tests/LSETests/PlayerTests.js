export function TestPlayer(players) {
    for (let player of players) {
        GetFromViewVector(player)
    }
}

function GetFromViewVector(player) {
    let en = player.getEntityFromViewVector(5.25)
    if (en) {
        logger.info(`Entity: ${en.name}`)
        let bl = player.getBlockFromViewVector(true, false, player.distanceTo(en), false)
        if (bl) {
            logger.info(`Block: ${bl.name} ${bl.pos}`)
        } else {
            logger.info(`No block found`)
        }
    } else {
        logger.info(`No entity found`)
        let bl = player.getBlockFromViewVector(true, false, 5.25, false)
        if (bl) {
            logger.info(`Block: ${bl.name} ${bl.pos}`)
        } else {
            logger.info(`No block found`)
        }
    }
}