/**
 * Combat & Weapons System Handlers (Phase 15)
 *
 * Complete combat implementation including:
 * - Weapon Base (creation, mesh, sockets, stats)
 * - Firing Modes (hitscan, projectile, spread, recoil, ADS)
 * - Projectiles (creation, movement, collision, homing)
 * - Damage System (damage types, execution, hitboxes)
 * - Weapon Features (reload, ammo, attachments, switching)
 * - Effects (muzzle flash, tracer, impact, shell ejection)
 * - Melee Combat (traces, combos, hitstop, hit reactions, parry, block, trails)
 *
 * @module combat-handlers
 */

import { ITools } from '../../types/tool-interfaces.js';
import { cleanObject } from '../../utils/safe-json.js';
import type { HandlerArgs } from '../../types/handler-types.js';
import { requireNonEmptyString, executeAutomationRequest } from './common-handlers.js';

function getTimeoutMs(): number {
  const envDefault = Number(process.env.MCP_AUTOMATION_REQUEST_TIMEOUT_MS ?? '120000');
  return Number.isFinite(envDefault) && envDefault > 0 ? envDefault : 120000;
}

/**
 * Handles all combat & weapon actions for the manage_combat tool.
 */
export async function handleCombatTools(
  action: string,
  args: HandlerArgs,
  tools: ITools
): Promise<Record<string, unknown>> {
  const argsRecord = args as Record<string, unknown>;
  const timeoutMs = getTimeoutMs();

  // All actions are dispatched to C++ via automation bridge
  const sendRequest = async (subAction: string): Promise<Record<string, unknown>> => {
    const payload = { ...argsRecord, subAction };
    const result = await executeAutomationRequest(
      tools,
      'manage_combat',
      payload as HandlerArgs,
      `Automation bridge not available for combat action: ${subAction}`,
      { timeoutMs }
    );
    return cleanObject(result) as Record<string, unknown>;
  };

  switch (action) {
    // =========================================================================
    // 15.1 Weapon Base (4 actions)
    // =========================================================================

    case 'create_weapon_blueprint': {
      requireNonEmptyString(argsRecord.name, 'name', 'Missing required parameter: name');
      return sendRequest('create_weapon_blueprint');
    }

    case 'configure_weapon_mesh': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_weapon_mesh');
    }

    case 'configure_weapon_sockets': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_weapon_sockets');
    }

    case 'set_weapon_stats': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('set_weapon_stats');
    }

    // =========================================================================
    // 15.2 Firing Modes (5 actions)
    // =========================================================================

    case 'configure_hitscan': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_hitscan');
    }

    case 'configure_projectile': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_projectile');
    }

    case 'configure_spread_pattern': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_spread_pattern');
    }

    case 'configure_recoil_pattern': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_recoil_pattern');
    }

    case 'configure_aim_down_sights': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_aim_down_sights');
    }

    // =========================================================================
    // 15.3 Projectiles (4 actions)
    // =========================================================================

    case 'create_projectile_blueprint': {
      requireNonEmptyString(argsRecord.name, 'name', 'Missing required parameter: name');
      return sendRequest('create_projectile_blueprint');
    }

    case 'configure_projectile_movement': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_projectile_movement');
    }

    case 'configure_projectile_collision': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_projectile_collision');
    }

    case 'configure_projectile_homing': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_projectile_homing');
    }

    // =========================================================================
    // 15.4 Damage System (3 actions)
    // =========================================================================

    case 'create_damage_type': {
      requireNonEmptyString(argsRecord.name, 'name', 'Missing required parameter: name');
      return sendRequest('create_damage_type');
    }

    case 'configure_damage_execution': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_damage_execution');
    }

    case 'setup_hitbox_component': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('setup_hitbox_component');
    }

    // =========================================================================
    // 15.5 Weapon Features (4 actions)
    // =========================================================================

    case 'setup_reload_system': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('setup_reload_system');
    }

    case 'setup_ammo_system': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('setup_ammo_system');
    }

    case 'setup_attachment_system': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('setup_attachment_system');
    }

    case 'setup_weapon_switching': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('setup_weapon_switching');
    }

    // =========================================================================
    // 15.6 Effects (4 actions)
    // =========================================================================

    case 'configure_muzzle_flash': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_muzzle_flash');
    }

    case 'configure_tracer': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_tracer');
    }

    case 'configure_impact_effects': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_impact_effects');
    }

    case 'configure_shell_ejection': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_shell_ejection');
    }

    // =========================================================================
    // 15.7 Melee Combat (6 actions)
    // =========================================================================

    case 'create_melee_trace': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('create_melee_trace');
    }

    case 'configure_combo_system': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_combo_system');
    }

    case 'create_hit_pause': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('create_hit_pause');
    }

    case 'configure_hit_reaction': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_hit_reaction');
    }

    case 'setup_parry_block_system': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('setup_parry_block_system');
    }

    case 'configure_weapon_trails': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('configure_weapon_trails');
    }

    // =========================================================================
    // Utility (1 action)
    // =========================================================================

    case 'get_combat_info': {
      requireNonEmptyString(argsRecord.blueprintPath, 'blueprintPath', 'Missing required parameter: blueprintPath');
      return sendRequest('get_combat_info');
    }

    // =========================================================================
    // Default / Unknown Action
    // =========================================================================

    default:
      return cleanObject({
        success: false,
        error: 'UNKNOWN_ACTION',
        message: `Unknown combat action: ${action}`
      });
  }
}

/**
 * Handles all combat stats actions for the manage_combat_stats tool.
 * This includes stat definitions, damage types, status effects, and formulas.
 */
export async function handleCombatStatsTools(
  action: string,
  args: HandlerArgs,
  tools: ITools
): Promise<Record<string, unknown>> {
  const argsRecord = args as Record<string, unknown>;
  const timeoutMs = Number(process.env.MCP_AUTOMATION_REQUEST_TIMEOUT_MS ?? '120000');

  const sendRequest = async (subAction: string): Promise<Record<string, unknown>> => {
    const payload = { ...argsRecord, subAction };
    const result = await executeAutomationRequest(
      tools,
      'manage_combat_stats',
      payload as HandlerArgs,
      `Automation bridge not available for combat stats action: ${subAction}`,
      { timeoutMs }
    );
    return cleanObject(result) as Record<string, unknown>;
  };

  switch (action) {
    // =========================================================================
    // Stat Definitions
    // =========================================================================

    case 'create_stat_definition': {
      requireNonEmptyString(argsRecord.statId, 'statId', 'Missing required parameter: statId');
      requireNonEmptyString(argsRecord.statName, 'statName', 'Missing required parameter: statName');
      return sendRequest('create_stat_definition');
    }

    case 'configure_derived_stat': {
      requireNonEmptyString(argsRecord.statId, 'statId', 'Missing required parameter: statId');
      requireNonEmptyString(argsRecord.formula, 'formula', 'Missing required parameter: formula');
      return sendRequest('configure_derived_stat');
    }

    case 'set_stat_bounds': {
      requireNonEmptyString(argsRecord.statId, 'statId', 'Missing required parameter: statId');
      return sendRequest('set_stat_bounds');
    }

    case 'add_stat_modifier': {
      requireNonEmptyString(argsRecord.targetActor, 'targetActor', 'Missing required parameter: targetActor');
      requireNonEmptyString(argsRecord.statId, 'statId', 'Missing required parameter: statId');
      return sendRequest('add_stat_modifier');
    }

    case 'remove_stat_modifier': {
      requireNonEmptyString(argsRecord.targetActor, 'targetActor', 'Missing required parameter: targetActor');
      requireNonEmptyString(argsRecord.modifierId, 'modifierId', 'Missing required parameter: modifierId');
      return sendRequest('remove_stat_modifier');
    }

    // =========================================================================
    // Damage Types
    // =========================================================================

    case 'create_damage_type': {
      requireNonEmptyString(argsRecord.damageTypeId, 'damageTypeId', 'Missing required parameter: damageTypeId');
      requireNonEmptyString(argsRecord.damageTypeName, 'damageTypeName', 'Missing required parameter: damageTypeName');
      return sendRequest('create_damage_type');
    }

    case 'configure_damage_resistance': {
      requireNonEmptyString(argsRecord.damageTypeId, 'damageTypeId', 'Missing required parameter: damageTypeId');
      return sendRequest('configure_damage_resistance');
    }

    case 'set_damage_multiplier': {
      requireNonEmptyString(argsRecord.damageTypeId, 'damageTypeId', 'Missing required parameter: damageTypeId');
      return sendRequest('set_damage_multiplier');
    }

    // =========================================================================
    // Status Effects
    // =========================================================================

    case 'create_status_effect': {
      requireNonEmptyString(argsRecord.effectId, 'effectId', 'Missing required parameter: effectId');
      requireNonEmptyString(argsRecord.effectName, 'effectName', 'Missing required parameter: effectName');
      return sendRequest('create_status_effect');
    }

    case 'configure_effect_stacking': {
      requireNonEmptyString(argsRecord.effectId, 'effectId', 'Missing required parameter: effectId');
      return sendRequest('configure_effect_stacking');
    }

    case 'set_effect_visuals': {
      requireNonEmptyString(argsRecord.effectId, 'effectId', 'Missing required parameter: effectId');
      return sendRequest('set_effect_visuals');
    }

    case 'apply_status_effect': {
      requireNonEmptyString(argsRecord.targetActor, 'targetActor', 'Missing required parameter: targetActor');
      requireNonEmptyString(argsRecord.effectId, 'effectId', 'Missing required parameter: effectId');
      return sendRequest('apply_status_effect');
    }

    case 'remove_status_effect': {
      requireNonEmptyString(argsRecord.targetActor, 'targetActor', 'Missing required parameter: targetActor');
      requireNonEmptyString(argsRecord.effectId, 'effectId', 'Missing required parameter: effectId');
      return sendRequest('remove_status_effect');
    }

    case 'clear_all_effects': {
      requireNonEmptyString(argsRecord.targetActor, 'targetActor', 'Missing required parameter: targetActor');
      return sendRequest('clear_all_effects');
    }

    // =========================================================================
    // Damage Formulas
    // =========================================================================

    case 'configure_damage_formula': {
      requireNonEmptyString(argsRecord.formulaId, 'formulaId', 'Missing required parameter: formulaId');
      requireNonEmptyString(argsRecord.formula, 'formula', 'Missing required parameter: formula');
      return sendRequest('configure_damage_formula');
    }

    case 'set_critical_hit_formula': {
      return sendRequest('set_critical_hit_formula');
    }

    case 'configure_armor_formula': {
      return sendRequest('configure_armor_formula');
    }

    case 'set_healing_formula': {
      return sendRequest('set_healing_formula');
    }

    // =========================================================================
    // Combat AI Profiles
    // =========================================================================

    case 'create_combat_ai_profile': {
      requireNonEmptyString(argsRecord.profileId, 'profileId', 'Missing required parameter: profileId');
      requireNonEmptyString(argsRecord.profileName, 'profileName', 'Missing required parameter: profileName');
      return sendRequest('create_combat_ai_profile');
    }

    case 'configure_ai_targeting': {
      requireNonEmptyString(argsRecord.profileId, 'profileId', 'Missing required parameter: profileId');
      return sendRequest('configure_ai_targeting');
    }

    case 'configure_ai_abilities': {
      requireNonEmptyString(argsRecord.profileId, 'profileId', 'Missing required parameter: profileId');
      return sendRequest('configure_ai_abilities');
    }

    case 'set_ai_positioning': {
      requireNonEmptyString(argsRecord.profileId, 'profileId', 'Missing required parameter: profileId');
      return sendRequest('set_ai_positioning');
    }

    // =========================================================================
    // Query Actions
    // =========================================================================

    case 'get_stat_value': {
      requireNonEmptyString(argsRecord.targetActor, 'targetActor', 'Missing required parameter: targetActor');
      requireNonEmptyString(argsRecord.statId, 'statId', 'Missing required parameter: statId');
      return sendRequest('get_stat_value');
    }

    case 'get_all_stats': {
      requireNonEmptyString(argsRecord.targetActor, 'targetActor', 'Missing required parameter: targetActor');
      return sendRequest('get_all_stats');
    }

    case 'get_active_effects': {
      requireNonEmptyString(argsRecord.targetActor, 'targetActor', 'Missing required parameter: targetActor');
      return sendRequest('get_active_effects');
    }

    case 'list_damage_types': {
      return sendRequest('list_damage_types');
    }

    case 'list_status_effects': {
      return sendRequest('list_status_effects');
    }

    case 'list_combat_ai_profiles': {
      return sendRequest('list_combat_ai_profiles');
    }

    // =========================================================================
    // Default / Unknown Action
    // =========================================================================

    default:
      return cleanObject({
        success: false,
        error: 'UNKNOWN_ACTION',
        message: `Unknown combat stats action: ${action}`
      });
  }
}
