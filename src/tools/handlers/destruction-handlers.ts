/**
 * FORGE2 Destruction System Handlers
 * Handles destructible environments, debris, repair, and hazards
 */

import { ITools } from '../../types/tool-interfaces.js';
import {
  Forge2Categories,
  executeForge2Action
} from './utils/forge2-bridge.js';
import type { Forge2Response } from './utils/response.js';

export async function handleDestructionTools(
    action: string,
    args: Record<string, unknown>,
    tools: ITools
): Promise<Forge2Response> {
    const params = { ...args, action };

    switch (action) {
        // Destructible Buildings
        case 'create_destructible_building':
        case 'configure_progressive_damage':
        case 'setup_damage_thresholds':
        case 'configure_destruction_levels':
            return await executeForge2Action(
                tools,
                Forge2Categories.DESTRUCTION,
                action,
                params
            );

        // Debris System
        case 'configure_debris_system':
        case 'setup_physics_debris':
        case 'configure_debris_cleanup':
        case 'setup_debris_pooling':
            return await executeForge2Action(
                tools,
                Forge2Categories.DESTRUCTION,
                action,
                params
            );

        // Repair System
        case 'setup_repair_system':
        case 'configure_rebuilding':
        case 'configure_restoration':
        case 'setup_repair_costs':
            return await executeForge2Action(
                tools,
                Forge2Categories.DESTRUCTION,
                action,
                params
            );

        // Environmental Hazards
        case 'create_environmental_hazards':
        case 'setup_fire_spread':
        case 'setup_flooding':
        case 'configure_hazard_damage':
            return await executeForge2Action(
                tools,
                Forge2Categories.DESTRUCTION,
                action,
                params
            );

        // Structural Integrity
        case 'configure_structural_integrity':
        case 'setup_support_beams':
        case 'configure_collapse_physics':
        case 'setup_chain_destruction':
            return await executeForge2Action(
                tools,
                Forge2Categories.DESTRUCTION,
                action,
                params
            );

        default:
            return {
                success: false,
                error: `Unknown destruction action: ${action}`,
                details: {
                    availableActions: [
                        'create_destructible_building', 'configure_progressive_damage',
                        'configure_debris_system', 'setup_repair_system',
                        'create_environmental_hazards', 'setup_fire_spread',
                        'configure_structural_integrity', 'setup_chain_destruction'
                    ]
                }
            };
    }
}
