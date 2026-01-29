/**
 * FORGE2 Damage System Handlers
 * Handles damage types, calculations, feedback, and death handling
 */

import { ITools } from '../../types/tool-interfaces.js';
import {
  Forge2Categories,
  executeForge2Action
} from './utils/forge2-bridge.js';
import type { Forge2Response } from './utils/response.js';

export async function handleDamageTools(
    action: string,
    args: Record<string, unknown>,
    tools: ITools
): Promise<Forge2Response> {
    const params = { ...args, action };

    switch (action) {
        // Damage Types
        case 'create_damage_type':
        case 'define_physical_damage':
        case 'define_magical_damage':
        case 'define_fire_damage':
        case 'define_ice_damage':
        case 'define_lightning_damage':
        case 'define_poison_damage':
        case 'define_holy_damage':
        case 'define_dark_damage':
            return await executeForge2Action(
                tools,
                Forge2Categories.DAMAGE,
                action,
                params
            );

        // Damage Calculations
        case 'configure_damage_falloff':
        case 'setup_distance_falloff':
        case 'setup_time_falloff':
        case 'setup_penetration_falloff':
        case 'setup_critical_hits':
        case 'configure_critical_chance':
        case 'configure_critical_multiplier':
        case 'configure_critical_zones':
            return await executeForge2Action(
                tools,
                Forge2Categories.DAMAGE,
                action,
                params
            );

        // Damage Feedback
        case 'create_damage_numbers':
        case 'configure_floating_text':
        case 'configure_damage_colors':
        case 'setup_hit_marker':
            return await executeForge2Action(
                tools,
                Forge2Categories.DAMAGE,
                action,
                params
            );

        // Death & Respawn
        case 'configure_death_handling':
        case 'setup_respawn_system':
        case 'configure_permadeath':
        case 'setup_checkpoint_respawn':
        case 'setup_invincibility_frames':
        case 'configure_iframe_duration':
        case 'configure_iframe_triggers':
            return await executeForge2Action(
                tools,
                Forge2Categories.DAMAGE,
                action,
                params
            );

        default:
            return {
                success: false,
                error: `Unknown damage action: ${action}`,
                details: {
                    availableActions: [
                        'create_damage_type', 'define_physical_damage', 'define_magical_damage',
                        'define_fire_damage', 'define_ice_damage', 'configure_damage_falloff',
                        'setup_critical_hits', 'create_damage_numbers', 'configure_death_handling',
                        'setup_respawn_system', 'setup_invincibility_frames'
                    ]
                }
            };
    }
}
