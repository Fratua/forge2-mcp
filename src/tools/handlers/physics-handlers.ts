/**
 * FORGE2 Physics System Handlers
 * Handles physics materials, constraints, volumes, ragdoll, and projectiles
 */

import { ITools } from '../../types/tool-interfaces.js';
import {
  Forge2Categories,
  executeForge2Action
} from './utils/forge2-bridge.js';
import type { Forge2Response } from './utils/response.js';

export async function handlePhysicsTools(
    action: string,
    args: Record<string, unknown>,
    tools: ITools
): Promise<Forge2Response> {
    const params = { ...args, action };

    switch (action) {
        // Physics Materials
        case 'create_physics_material':
        case 'configure_friction':
        case 'configure_restitution':
        case 'configure_density':
            return await executeForge2Action(
                tools,
                Forge2Categories.PHYSICS,
                action,
                params
            );

        // Physics Constraints
        case 'configure_physics_constraints':
        case 'create_hinge_joint':
        case 'create_ball_socket_joint':
        case 'create_prismatic_joint':
        case 'configure_joint_limits':
        case 'configure_joint_motors':
            return await executeForge2Action(
                tools,
                Forge2Categories.PHYSICS,
                action,
                params
            );

        // Physics Volumes
        case 'setup_physics_volumes':
        case 'create_water_volume':
        case 'create_low_gravity_volume':
        case 'create_wind_volume':
        case 'create_kill_volume':
            return await executeForge2Action(
                tools,
                Forge2Categories.PHYSICS,
                action,
                params
            );

        // Ragdoll
        case 'create_ragdoll':
        case 'configure_ragdoll_constraints':
        case 'setup_ragdoll_blend':
        case 'configure_ragdoll_recovery':
        case 'setup_hit_reaction_impulse':
            return await executeForge2Action(
                tools,
                Forge2Categories.PHYSICS,
                action,
                params
            );

        // Projectile Physics
        case 'setup_projectile_physics':
        case 'configure_trajectory':
        case 'configure_gravity_effect':
        case 'configure_penetration':
        case 'configure_ricochet':
            return await executeForge2Action(
                tools,
                Forge2Categories.PHYSICS,
                action,
                params
            );

        default:
            return {
                success: false,
                error: `Unknown physics action: ${action}`,
                details: {
                    availableActions: [
                        'create_physics_material', 'configure_friction', 'configure_restitution',
                        'configure_physics_constraints', 'create_hinge_joint', 'create_ball_socket_joint',
                        'setup_physics_volumes', 'create_water_volume', 'create_ragdoll',
                        'setup_projectile_physics', 'configure_trajectory'
                    ]
                }
            };
    }
}
