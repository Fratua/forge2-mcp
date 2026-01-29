/**
 * FORGE2 Camera System Handlers
 * Handles camera modes, collision, shake, and transitions
 */

import { ITools } from '../../types/tool-interfaces.js';
import {
  Forge2Categories,
  executeForge2Action
} from './utils/forge2-bridge.js';
import type { Forge2Response } from './utils/response.js';

export async function handleCameraTools(
    action: string,
    args: Record<string, unknown>,
    tools: ITools
): Promise<Forge2Response> {
    const params = { ...args, action };

    switch (action) {
        // Camera Modes
        case 'create_camera_mode':
        case 'configure_third_person':
        case 'configure_first_person':
        case 'configure_isometric':
        case 'configure_side_scroller':
        case 'configure_top_down':
        case 'configure_orbit':
            return await executeForge2Action(
                tools,
                Forge2Categories.CAMERA,
                action,
                params
            );

        // Camera Collision
        case 'configure_camera_collision':
        case 'setup_sphere_sweep':
        case 'configure_camera_push':
        case 'configure_camera_cut':
        case 'setup_transparent_occlusion':
            return await executeForge2Action(
                tools,
                Forge2Categories.CAMERA,
                action,
                params
            );

        // Camera Shake
        case 'setup_camera_shake':
        case 'create_perlin_shake':
        case 'create_oscillation_shake':
        case 'create_impact_shake':
        case 'configure_shake_attenuation':
            return await executeForge2Action(
                tools,
                Forge2Categories.CAMERA,
                action,
                params
            );

        // Camera Lag & Constraints
        case 'configure_camera_lag':
        case 'setup_position_lag':
        case 'setup_rotation_lag':
        case 'setup_zoom_lag':
        case 'setup_camera_constraints':
        case 'configure_pitch_limits':
        case 'configure_distance_limits':
            return await executeForge2Action(
                tools,
                Forge2Categories.CAMERA,
                action,
                params
            );

        // Camera Transitions
        case 'create_camera_transition':
        case 'setup_blend_transition':
        case 'setup_cut_transition':
        case 'configure_ease_curves':
            return await executeForge2Action(
                tools,
                Forge2Categories.CAMERA,
                action,
                params
            );

        // Split Screen
        case 'configure_split_screen':
        case 'setup_horizontal_split':
        case 'setup_vertical_split':
        case 'setup_dynamic_split':
            return await executeForge2Action(
                tools,
                Forge2Categories.CAMERA,
                action,
                params
            );

        default:
            return {
                success: false,
                error: `Unknown camera action: ${action}`,
                details: {
                    availableActions: [
                        'create_camera_mode', 'configure_third_person', 'configure_first_person',
                        'configure_isometric', 'configure_side_scroller', 'configure_top_down',
                        'configure_orbit', 'configure_camera_collision', 'setup_camera_shake',
                        'configure_camera_lag', 'setup_camera_constraints', 'create_camera_transition',
                        'configure_split_screen'
                    ]
                }
            };
    }
}
