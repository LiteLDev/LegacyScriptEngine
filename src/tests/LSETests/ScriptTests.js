export function TestLogger(players) {
    for (let player of players) {
        logger.setPlayer(player)
    }
    logger.setFile("logs/LegacyScriptEngine/ScriptTests.log");
    logger.log("This is a log message");
    logger.warn("This is a warning message");
    logger.error("This is an error message");
    logger.debug("This is a debug message");
    logger.info("This is an info message");
    logger.fatal("This is a fetal message");
}