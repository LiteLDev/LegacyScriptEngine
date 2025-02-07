import {RegisterEvents, events, triggeredEvents} from './plugins/LSETests/EventTests.js';
import {TestLogger} from './plugins/LSETests/ScriptTests.js';
import {TestPlayer} from './plugins/LSETests/PlayerTests.js';

RegisterEvents();

mc.listen('onServerStarted', () => {
    let cmd = mc.newCommand('lsetests', "LegacyScriptEngine tests", PermType.Console);
    cmd.setEnum('testOption', ['logger', 'events', 'player']);
    cmd.mandatory('testOption', ParamType.Enum, 'testOption');
    cmd.optional('player', ParamType.Player);
    cmd.overload('testOption', 'player');
    cmd.setCallback((cmd, origin, output, results) => {
        switch (results.testOption) {
            case 'logger':
                TestLogger(results.player);
                break;
            case 'events':
                const
                    notTriggeredEvents = events.filter(event => !triggeredEvents.has(event));
                logger.info(`Events not triggered: ${notTriggeredEvents.join(', ')}`);
                break;
            case 'player':
                TestPlayer(results.player);
                break;
            default:
                logger.error(`Invalid test option ${results.testOption}`);
                break;
        }
    });
})

