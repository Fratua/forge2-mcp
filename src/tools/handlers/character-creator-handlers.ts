/**
 * FORGE2 Character Creator System Handlers
 * Handles character customization including body morphs, face, hair, skin, and accessories
 */

import { ITools } from '../../types/tool-interfaces.js';
import {
  Forge2Categories,
  executeForge2Action
} from './utils/forge2-bridge.js';
import type { Forge2Response } from './utils/response.js';

export async function handleCharacterCreatorTools(
    action: string,
    args: Record<string, unknown>,
    tools: ITools
): Promise<Forge2Response> {
    const params = { ...args, action };

    switch (action) {
        // Body Morph System
        case 'create_body_morph_system':
        case 'configure_height':
        case 'configure_weight':
        case 'configure_proportions':
        case 'setup_body_presets':
            return await executeForge2Action(
                tools,
                Forge2Categories.CHARACTER_CREATOR,
                action,
                params
            );

        // Face Customization
        case 'configure_face_customization':
        case 'setup_face_sliders':
        case 'create_face_presets':
        case 'configure_facial_features':
            return await executeForge2Action(
                tools,
                Forge2Categories.CHARACTER_CREATOR,
                action,
                params
            );

        // Hair System
        case 'setup_hair_system':
        case 'configure_hairstyles':
        case 'configure_hair_colors':
        case 'setup_hair_physics':
            return await executeForge2Action(
                tools,
                Forge2Categories.CHARACTER_CREATOR,
                action,
                params
            );

        // Skin System
        case 'create_skin_system':
        case 'configure_skin_tones':
        case 'configure_skin_textures':
        case 'setup_body_markings':
            return await executeForge2Action(
                tools,
                Forge2Categories.CHARACTER_CREATOR,
                action,
                params
            );

        // Clothing System
        case 'configure_clothing_system':
        case 'setup_layered_clothing':
        case 'configure_dyeable_clothing':
        case 'setup_clothing_physics':
            return await executeForge2Action(
                tools,
                Forge2Categories.CHARACTER_CREATOR,
                action,
                params
            );

        // Accessory System
        case 'setup_accessory_system':
        case 'configure_jewelry':
        case 'configure_tattoos':
        case 'configure_scars':
            return await executeForge2Action(
                tools,
                Forge2Categories.CHARACTER_CREATOR,
                action,
                params
            );

        default:
            return {
                success: false,
                error: `Unknown character creator action: ${action}`,
                details: {
                    availableActions: [
                        'create_body_morph_system', 'configure_height', 'configure_weight',
                        'configure_face_customization', 'setup_face_sliders',
                        'setup_hair_system', 'configure_hairstyles',
                        'create_skin_system', 'configure_skin_tones',
                        'configure_clothing_system', 'setup_accessory_system'
                    ]
                }
            };
    }
}
