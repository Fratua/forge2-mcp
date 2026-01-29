import { commonSchemas } from './tool-definition-utils.js';
/** MCP Tool Definition type for explicit annotation to avoid TS7056 */
export interface ToolDefinition {
  category?: 'core' | 'world' | 'authoring' | 'gameplay' | 'utility' | 'genre' | 'context';
  name: string;
  description: string;
  inputSchema: Record<string, unknown>;
  outputSchema?: Record<string, unknown>;
  [key: string]: unknown;
}
export const consolidatedToolDefinitions: ToolDefinition[] = [
  {
    name: 'manage_pipeline',
    description: 'Filter visible tools by category. Actions: list_categories (show available), set_categories (enable specific), get_status (current state). Categories: core, world, authoring, gameplay, utility, all.',
    category: 'core',
    inputSchema: {
      type: 'object',
      properties: {
        action: { type: 'string', enum: ['set_categories', 'list_categories', 'get_status'], description: 'list_categories: show available. set_categories: enable categories. get_status: current state.' },
        categories: { type: 'array', items: commonSchemas.stringProp, description: 'Categories: core, world, authoring, gameplay, utility, all' }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        categories: { type: 'array', items: commonSchemas.stringProp }
      }
    }
  },
  {
    name: 'manage_asset',
    category: 'core',
    description: 'Create, import, duplicate, rename, delete assets. Edit Material graphs and instances. Analyze dependencies.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'list', 'import', 'duplicate', 'rename', 'move', 'delete', 'delete_asset', 'delete_assets', 'create_folder', 'search_assets',
            'get_dependencies', 'get_source_control_state', 'analyze_graph', 'get_asset_graph', 'create_thumbnail', 'set_tags', 'get_metadata', 'set_metadata', 'validate', 'fixup_redirectors', 'find_by_tag', 'generate_report',
            'create_material', 'create_material_instance', 'create_render_target', 'generate_lods', 'add_material_parameter', 'list_instances', 'reset_instance_parameters', 'exists', 'get_material_stats',
            'nanite_rebuild_mesh', 'bulk_rename', 'bulk_delete', 'source_control_checkout', 'source_control_submit',
            'add_material_node', 'connect_material_pins', 'remove_material_node', 'break_material_connections', 'get_material_node_details', 'rebuild_material'
          ],
          description: 'Action to perform'
        },
        assetPath: commonSchemas.assetPath,
        directory: commonSchemas.directoryPath,
        classNames: commonSchemas.arrayOfStrings,
        packagePaths: commonSchemas.arrayOfStrings,
        recursivePaths: commonSchemas.booleanProp,
        recursiveClasses: commonSchemas.booleanProp,
        limit: commonSchemas.numberProp,
        sourcePath: commonSchemas.sourcePath,
        destinationPath: commonSchemas.destinationPath,
        assetPaths: commonSchemas.arrayOfStrings,
        lodCount: commonSchemas.numberProp,
        reductionSettings: commonSchemas.objectProp,
        nodeName: commonSchemas.name,
        eventName: commonSchemas.eventName,
        memberClass: commonSchemas.stringProp,
        posX: commonSchemas.numberProp,
        posY: commonSchemas.numberProp,
        newName: commonSchemas.newName,
        overwrite: commonSchemas.overwrite,
        save: commonSchemas.save,
        fixupRedirectors: commonSchemas.booleanProp,
        directoryPath: commonSchemas.directoryPath,
        name: commonSchemas.name,
        path: commonSchemas.directoryPath,
        parentMaterial: commonSchemas.materialPath,
        parameters: commonSchemas.objectProp,
        width: commonSchemas.numberProp,
        height: commonSchemas.numberProp,
        format: commonSchemas.stringProp,
        meshPath: commonSchemas.meshPath,
        tag: commonSchemas.tagName,
        metadata: commonSchemas.objectProp,
        graphName: commonSchemas.graphName,
        nodeType: commonSchemas.stringProp,
        nodeId: commonSchemas.nodeId,
        sourceNodeId: commonSchemas.sourceNodeId,
        targetNodeId: commonSchemas.targetNodeId,
        inputName: commonSchemas.pinName,
        fromNodeId: commonSchemas.sourceNodeId,
        fromPin: commonSchemas.sourcePin,
        toNodeId: commonSchemas.targetNodeId,
        toPin: commonSchemas.targetPin,
        parameterName: commonSchemas.parameterName,
        value: commonSchemas.value,
        x: commonSchemas.numberProp,
        y: commonSchemas.numberProp,
        comment: commonSchemas.stringProp,
        parentNodeId: commonSchemas.nodeId,
        childNodeId: commonSchemas.nodeId,
        maxDepth: commonSchemas.numberProp,
        // Bulk operations (C++ TryGetStringField)
        prefix: commonSchemas.stringProp,
        suffix: commonSchemas.stringProp,
        searchText: commonSchemas.stringProp,
        replaceText: commonSchemas.stringProp,
        paths: commonSchemas.arrayOfStrings,
        // Source control (C++ TryGetStringField)
        description: commonSchemas.stringProp,
        checkoutFiles: commonSchemas.booleanProp,
        // Bulk delete
        showConfirmation: commonSchemas.booleanProp,
        // Material graph operations
        pinName: commonSchemas.pinName,
        desc: commonSchemas.stringProp,
        materialPath: commonSchemas.materialPath,
        texturePath: commonSchemas.texturePath,
        expressionClass: commonSchemas.stringProp,
        coordinateIndex: commonSchemas.numberProp,
        parameterType: commonSchemas.stringProp,
        nodes: commonSchemas.arrayOfObjects,
        tags: commonSchemas.arrayOfStrings
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assets: commonSchemas.arrayOfObjects,
        paths: commonSchemas.arrayOfStrings,
        path: commonSchemas.stringProp,
        nodeId: commonSchemas.nodeId,
        details: commonSchemas.objectProp
      }
    }
  },
  {
    name: 'manage_blueprint',
    category: 'authoring',
    description: 'Create Blueprints, add SCS components (mesh, collision, camera), and manipulate graph nodes.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create', 'get_blueprint', 'get', 'compile',
            'add_component', 'set_default', 'modify_scs', 'get_scs', 'add_scs_component', 'remove_scs_component', 'reparent_scs_component', 'set_scs_transform', 'set_scs_property',
            'ensure_exists', 'probe_handle', 'add_variable', 'remove_variable', 'rename_variable', 'add_function', 'add_event', 'remove_event', 'add_construction_script', 'set_variable_metadata', 'set_metadata',
            'create_node', 'add_node', 'delete_node', 'connect_pins', 'break_pin_links', 'set_node_property', 'create_reroute_node', 'get_node_details', 'get_graph_details', 'get_pin_details',
            'list_node_types', 'set_pin_default_value'
          ],
          description: 'Blueprint action'
        },
        name: commonSchemas.name,
        blueprintPath: commonSchemas.blueprintPath,
        blueprintType: commonSchemas.parentClass,
        savePath: commonSchemas.savePath,
        componentType: commonSchemas.stringProp,
        componentName: commonSchemas.componentName,
        componentClass: commonSchemas.stringProp,
        attachTo: commonSchemas.stringProp,
        newParent: commonSchemas.stringProp,
        propertyName: commonSchemas.propertyName,
        variableName: commonSchemas.variableName,
        oldName: commonSchemas.stringProp,
        newName: commonSchemas.newName,
        value: commonSchemas.value,
        metadata: commonSchemas.objectProp,
        properties: commonSchemas.objectProp,
        graphName: commonSchemas.graphName,
        nodeType: commonSchemas.stringProp,
        nodeId: commonSchemas.nodeId,
        pinName: commonSchemas.pinName,
        linkedTo: commonSchemas.stringProp,
        memberName: commonSchemas.stringProp,
        x: commonSchemas.numberProp,
        y: commonSchemas.numberProp,
        location: commonSchemas.arrayOfNumbers,
        rotation: commonSchemas.arrayOfNumbers,
        scale: commonSchemas.arrayOfNumbers,
        operations: commonSchemas.arrayOfObjects,
        compile: commonSchemas.compile,
        save: commonSchemas.save,
        eventType: commonSchemas.stringProp,
        customEventName: commonSchemas.eventName,
        parameters: commonSchemas.arrayOfObjects,
        // Variable configuration (C++ TryGetStringField/BoolField)
        variableType: { type: 'string', description: 'Variable type (e.g., Boolean, Float, Integer, Vector, String, Object)' },
        defaultValue: commonSchemas.value,
        category: commonSchemas.stringProp,
        isReplicated: commonSchemas.booleanProp,
        isPublic: commonSchemas.booleanProp,
        variablePinType: commonSchemas.objectProp,
        // Function configuration
        functionName: commonSchemas.functionName,
        inputs: commonSchemas.arrayOfObjects,
        outputs: commonSchemas.arrayOfObjects,
        // Node positioning (C++ TryGetNumberField)
        posX: commonSchemas.numberProp,
        posY: commonSchemas.numberProp,
        // Event configuration
        eventName: commonSchemas.eventName,
        // Component/SCS configuration
        parentComponent: commonSchemas.stringProp,
        meshPath: commonSchemas.meshPath,
        materialPath: commonSchemas.materialPath,
        transform: commonSchemas.objectProp,
        applyAndSave: commonSchemas.booleanProp,
        // Script configuration
        scriptName: commonSchemas.stringProp,
        // Graph operations
        memberClass: commonSchemas.stringProp,
        targetClass: commonSchemas.stringProp,
        inputAxisName: commonSchemas.stringProp,
        inputPin: commonSchemas.pinName,
        outputPin: commonSchemas.pinName,
        // Compilation options
        saveAfterCompile: commonSchemas.booleanProp,
        // Timing/async options
        timeoutMs: commonSchemas.numberProp,
        waitForCompletion: commonSchemas.booleanProp,
        waitForCompletionTimeoutMs: commonSchemas.numberProp,
        // Parent class for blueprint creation
        parentClass: commonSchemas.parentClass,
        // Graph pin connections
        fromNodeId: commonSchemas.sourceNodeId,
        fromPin: commonSchemas.sourcePin,
        fromPinName: commonSchemas.sourcePin,
        toNodeId: commonSchemas.targetNodeId,
        toPin: commonSchemas.targetPin,
        toPinName: commonSchemas.targetPin
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        blueprintPath: commonSchemas.blueprintPath,
        blueprint: commonSchemas.objectProp
      }
    }
  },
  {
    name: 'control_actor',
    category: 'core',
    description: 'Spawn actors, set transforms, enable physics, add components, manage tags, and attach actors.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'spawn', 'spawn_blueprint', 'delete', 'delete_by_tag', 'duplicate',
            'apply_force', 'set_transform', 'get_transform', 'set_visibility',
            'add_component', 'set_component_properties', 'get_components',
            'add_tag', 'remove_tag', 'find_by_tag', 'find_by_name', 'list', 'set_blueprint_variables',
            'create_snapshot', 'attach', 'detach'
          ],
          description: 'Action'
        },
        actorName: commonSchemas.actorName,
        childActor: commonSchemas.childActorName,
        parentActor: commonSchemas.parentActorName,
        classPath: commonSchemas.assetPath,
        meshPath: commonSchemas.meshPath,
        blueprintPath: commonSchemas.blueprintPath,
        location: commonSchemas.location,
        rotation: commonSchemas.rotation,
        scale: commonSchemas.scale,
        force: commonSchemas.vector3,
        componentType: commonSchemas.stringProp,
        componentName: commonSchemas.componentName,
        properties: commonSchemas.objectProp,
        visible: commonSchemas.visible,
        newName: commonSchemas.newName,
        tag: commonSchemas.tagName,
        variables: commonSchemas.objectProp,
        snapshotName: commonSchemas.stringProp
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputWithActor,
        components: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              name: commonSchemas.stringProp,
              class: commonSchemas.stringProp,
              relativeLocation: commonSchemas.location,
              relativeRotation: commonSchemas.rotation,
              relativeScale: commonSchemas.scale
            }
          }
        },
        data: commonSchemas.objectProp
      }
    }
  },
  {
    name: 'control_editor',
    category: 'core',
    description: 'Start/stop PIE, control viewport camera, run console commands, take screenshots, simulate input.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'play', 'stop', 'stop_pie', 'pause', 'resume', 'set_game_speed', 'eject', 'possess',
            'set_camera', 'set_camera_position', 'set_camera_fov', 'set_view_mode',
            'set_viewport_resolution', 'console_command', 'execute_command', 'screenshot', 'step_frame',
            'start_recording', 'stop_recording', 'create_bookmark', 'jump_to_bookmark',
            'set_preferences', 'set_viewport_realtime', 'open_asset', 'simulate_input'
          ],
          description: 'Editor action'
        },
        location: commonSchemas.location,
        rotation: commonSchemas.rotation,
        viewMode: commonSchemas.stringProp,
        enabled: commonSchemas.enabled,
        speed: commonSchemas.numberProp,
        filename: commonSchemas.stringProp,
        fov: commonSchemas.numberProp,
        width: commonSchemas.numberProp,
        height: commonSchemas.numberProp,
        command: commonSchemas.stringProp,
        steps: commonSchemas.integerProp,
        bookmarkName: commonSchemas.stringProp,
        assetPath: commonSchemas.assetPath,
        keyName: commonSchemas.stringProp,
        eventType: { type: 'string', enum: ['KeyDown', 'KeyUp', 'Both'] }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase
      }
    }
  },
  {
    name: 'manage_level',
    category: 'core',
    description: 'Load/save levels, configure streaming, manage World Partition cells, and build lighting.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'load', 'save', 'save_as', 'save_level_as', 'stream', 'create_level', 'create_light', 'build_lighting',
            'set_metadata', 'load_cells', 'set_datalayer',
            'export_level', 'import_level', 'list_levels', 'get_summary', 'delete', 'validate_level',
            'cleanup_invalid_datalayers', 'add_sublevel'
          ],
          description: 'Action'
        },
        levelPath: commonSchemas.levelPath,
        levelName: commonSchemas.stringProp,
        streaming: commonSchemas.booleanProp,
        shouldBeLoaded: commonSchemas.booleanProp,
        shouldBeVisible: commonSchemas.booleanProp,
        lightType: { type: 'string', enum: ['Directional', 'Point', 'Spot', 'Rect'] },
        location: commonSchemas.location,
        intensity: commonSchemas.numberProp,
        quality: commonSchemas.stringProp,
        min: commonSchemas.arrayOfNumbers,
        max: commonSchemas.arrayOfNumbers,
        dataLayerLabel: commonSchemas.stringProp,
        dataLayerState: commonSchemas.stringProp,
        recursive: commonSchemas.recursive,
        exportPath: commonSchemas.outputPath,
        packagePath: commonSchemas.assetPath,
        destinationPath: commonSchemas.destinationPath,
        note: commonSchemas.stringProp,
        levelPaths: commonSchemas.arrayOfStrings,
        subLevelPath: commonSchemas.levelPath,
        parentLevel: commonSchemas.stringProp,
        streamingMethod: { type: 'string', enum: ['Blueprint', 'AlwaysLoaded'] }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        data: commonSchemas.objectProp
      }
    }
  },
  {
    name: 'animation_physics',
    category: 'utility',
    description: 'Create Animation BPs, Montages, Blend Spaces, IK rigs, ragdolls, vehicles, and author animation sequences/curves.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            // Runtime animation & physics
            'create_animation_bp', 'play_montage', 'setup_ragdoll', 'activate_ragdoll', 'configure_vehicle',
            'create_blend_space', 'create_state_machine', 'setup_ik', 'create_procedural_anim',
            'create_blend_tree', 'setup_retargeting', 'setup_physics_simulation', 'cleanup',
            'create_animation_asset', 'add_notify',
            // Animation authoring (merged from manage_animation_authoring)
            'create_animation_sequence', 'set_sequence_length', 'add_bone_track', 'set_bone_key', 'set_curve_key',
            'add_notify_state', 'add_sync_marker', 'set_root_motion_settings', 'set_additive_settings',
            'create_montage', 'add_montage_section', 'add_montage_slot', 'set_section_timing',
            'add_montage_notify', 'set_blend_in', 'set_blend_out', 'link_sections',
            'create_blend_space_1d', 'create_blend_space_2d', 'add_blend_sample', 'set_axis_settings', 'set_interpolation_settings',
            'create_aim_offset', 'add_aim_offset_sample',
            'create_anim_blueprint', 'add_state_machine', 'add_state', 'add_transition', 'set_transition_rules',
            'add_blend_node', 'add_cached_pose', 'add_slot_node', 'add_layered_blend_per_bone', 'set_anim_graph_node_value',
            'create_control_rig', 'add_control', 'add_rig_unit', 'connect_rig_elements', 'create_pose_library',
            'create_ik_rig', 'add_ik_chain', 'create_ik_retargeter', 'set_retarget_chain_mapping',
            'get_animation_info'
          ],
          description: 'Action'
        },
        name: commonSchemas.name,
        actorName: commonSchemas.actorName,
        skeletonPath: commonSchemas.skeletonPath,
        montagePath: commonSchemas.animationPath,
        animationPath: commonSchemas.animationPath,
        playRate: commonSchemas.numberProp,
        physicsAssetName: commonSchemas.stringProp,
        meshPath: commonSchemas.meshPath,
        vehicleName: commonSchemas.stringProp,
        vehicleType: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        // Authoring properties (merged from manage_animation_authoring)
        path: commonSchemas.directoryPathForCreation,
        assetPath: commonSchemas.assetPath,
        skeletalMeshPath: commonSchemas.skeletalMeshPath,
        blueprintPath: commonSchemas.blueprintPath,
        save: commonSchemas.save,
        numFrames: commonSchemas.numFrames,
        frameRate: commonSchemas.frameRate,
        boneName: commonSchemas.boneName,
        frame: commonSchemas.frame,
        location: commonSchemas.location,
        rotation: commonSchemas.rotation,
        scale: commonSchemas.scale,
        curveName: commonSchemas.curveName,
        value: commonSchemas.numberProp,
        createIfMissing: commonSchemas.booleanProp,
        notifyClass: commonSchemas.notifyClass,
        notifyName: commonSchemas.notifyName,
        trackIndex: commonSchemas.trackIndex,
        startFrame: commonSchemas.startFrame,
        endFrame: commonSchemas.endFrame,
        markerName: commonSchemas.markerName,
        enableRootMotion: commonSchemas.booleanProp,
        rootMotionRootLock: commonSchemas.stringProp,
        forceRootLock: commonSchemas.booleanProp,
        additiveAnimType: commonSchemas.stringProp,
        basePoseType: commonSchemas.stringProp,
        basePoseAnimation: commonSchemas.stringProp,
        basePoseFrame: commonSchemas.numberProp,
        slotName: commonSchemas.slotName,
        sectionName: commonSchemas.sectionName,
        startTime: commonSchemas.startTime,
        length: commonSchemas.numberProp,
        time: commonSchemas.numberProp,
        blendTime: commonSchemas.blendTime,
        blendOption: commonSchemas.stringProp,
        fromSection: commonSchemas.fromSection,
        toSection: commonSchemas.toSection,
        axisName: commonSchemas.axisName,
        axisMin: commonSchemas.minValue,
        axisMax: commonSchemas.maxValue,
        horizontalAxisName: commonSchemas.horizontalAxisName,
        horizontalMin: commonSchemas.numberProp,
        horizontalMax: commonSchemas.numberProp,
        verticalAxisName: commonSchemas.verticalAxisName,
        verticalMin: commonSchemas.numberProp,
        verticalMax: commonSchemas.numberProp,
        sampleValue: commonSchemas.value,
        axis: commonSchemas.stringProp,
        minValue: commonSchemas.numberProp,
        maxValue: commonSchemas.numberProp,
        gridDivisions: commonSchemas.numberProp,
        interpolationType: commonSchemas.stringProp,
        targetWeightInterpolationSpeed: commonSchemas.numberProp,
        yaw: commonSchemas.numberProp,
        pitch: commonSchemas.numberProp,
        parentClass: commonSchemas.parentClass,
        stateMachineName: commonSchemas.stateMachineName,
        stateName: commonSchemas.stateName,
        isEntryState: commonSchemas.booleanProp,
        fromState: commonSchemas.stringProp,
        toState: commonSchemas.stringProp,
        blendLogicType: commonSchemas.stringProp,
        automaticTriggerRule: commonSchemas.stringProp,
        automaticTriggerTime: commonSchemas.numberProp,
        blendType: commonSchemas.stringProp,
        nodeName: commonSchemas.nodeName,
        x: commonSchemas.nodeX,
        y: commonSchemas.nodeY,
        cacheName: commonSchemas.cacheName,
        layerSetup: commonSchemas.arrayOfObjects,
        propertyName: commonSchemas.propertyName,
        controlName: commonSchemas.controlName,
        controlType: commonSchemas.stringProp,
        parentBone: commonSchemas.parentBone,
        parentControl: commonSchemas.parentControl,
        unitType: commonSchemas.stringProp,
        unitName: commonSchemas.unitName,
        settings: commonSchemas.objectProp,
        sourceElement: commonSchemas.sourceElement,
        sourcePin: commonSchemas.sourcePin,
        targetElement: commonSchemas.targetElement,
        targetPin: commonSchemas.targetPin,
        chainName: commonSchemas.chainName,
        startBone: commonSchemas.startBone,
        endBone: commonSchemas.endBone,
        goal: commonSchemas.goal,
        sourceIKRigPath: commonSchemas.sourceIKRigPath,
        targetIKRigPath: commonSchemas.targetIKRigPath,
        sourceChain: commonSchemas.sourceChain,
        targetChain: commonSchemas.targetChain
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.assetPath,
        animationInfo: commonSchemas.objectProp
      }
    }
  },
  {
    name: 'manage_effect',
    category: 'utility',
    description: 'Spawn Niagara/Cascade particles, draw debug shapes, edit VFX node graphs, and author Niagara systems/emitters.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            // Runtime VFX control
            'particle', 'niagara', 'debug_shape', 'spawn_niagara', 'create_dynamic_light',
            'create_niagara_system', 'create_niagara_emitter',
            'create_volumetric_fog', 'create_particle_trail', 'create_environment_effect', 'create_impact_effect', 'create_niagara_ribbon',
            'activate', 'activate_effect', 'deactivate', 'reset', 'advance_simulation',
            'add_niagara_module', 'connect_niagara_pins', 'remove_niagara_node', 'set_niagara_parameter',
            'clear_debug_shapes', 'cleanup', 'list_debug_shapes',
            // Niagara authoring (merged from manage_niagara_authoring)
            'add_emitter_to_system', 'set_emitter_properties',
            'add_spawn_rate_module', 'add_spawn_burst_module', 'add_spawn_per_unit_module',
            'add_initialize_particle_module', 'add_particle_state_module',
            'add_force_module', 'add_velocity_module', 'add_acceleration_module',
            'add_size_module', 'add_color_module',
            'add_sprite_renderer_module', 'add_mesh_renderer_module', 'add_ribbon_renderer_module', 'add_light_renderer_module',
            'add_collision_module', 'add_kill_particles_module', 'add_camera_offset_module',
            'add_user_parameter', 'set_parameter_value', 'bind_parameter_to_source',
            'add_skeletal_mesh_data_interface', 'add_static_mesh_data_interface', 'add_spline_data_interface',
            'add_audio_spectrum_data_interface', 'add_collision_query_data_interface',
            'add_event_generator', 'add_event_receiver', 'configure_event_payload',
            'enable_gpu_simulation', 'add_simulation_stage',
            'get_niagara_info', 'validate_niagara_system'
          ],
          description: 'Action'
        },
        name: commonSchemas.name,
        systemName: commonSchemas.stringProp,
        systemPath: commonSchemas.niagaraPath,
        preset: { type: 'string', description: 'Path to particle system asset.' },
        location: commonSchemas.location,
        scale: commonSchemas.numberProp,
        shape: { type: 'string', description: 'Supported: sphere, box, cylinder, line, cone, capsule, arrow, plane' },
        size: commonSchemas.numberProp,
        color: commonSchemas.color,
        modulePath: commonSchemas.assetPath,
        emitterName: commonSchemas.stringProp,
        pinName: commonSchemas.pinName,
        linkedTo: commonSchemas.stringProp,
        parameterName: commonSchemas.parameterName,
        parameterType: commonSchemas.stringProp,
        type: commonSchemas.stringProp,
        value: commonSchemas.value,
        filter: commonSchemas.filter,
        // Authoring properties (merged from manage_niagara_authoring)
        path: commonSchemas.directoryPathForCreation,
        assetPath: commonSchemas.assetPath,
        emitterPath: commonSchemas.emitterPath,
        save: commonSchemas.save,
        emitterProperties: commonSchemas.objectProp,
        spawnRate: commonSchemas.numberProp,
        burstCount: commonSchemas.numberProp,
        burstTime: commonSchemas.numberProp,
        burstInterval: commonSchemas.numberProp,
        spawnPerUnit: commonSchemas.numberProp,
        lifetime: commonSchemas.numberProp,
        lifetimeMin: commonSchemas.numberProp,
        lifetimeMax: commonSchemas.numberProp,
        mass: commonSchemas.numberProp,
        spriteSize: commonSchemas.objectProp,
        meshScale: commonSchemas.objectProp,
        forceType: commonSchemas.stringProp,
        forceStrength: commonSchemas.numberProp,
        forceVector: commonSchemas.objectProp,
        dragCoefficient: commonSchemas.numberProp,
        velocity: commonSchemas.objectProp,
        velocityMin: commonSchemas.objectProp,
        velocityMax: commonSchemas.objectProp,
        acceleration: commonSchemas.objectProp,
        velocityMode: commonSchemas.stringProp,
        sizeMode: commonSchemas.stringProp,
        uniformSize: commonSchemas.numberProp,
        sizeScale: commonSchemas.objectProp,
        sizeCurve: commonSchemas.arrayOfObjects,
        colorMin: commonSchemas.objectProp,
        colorMax: commonSchemas.objectProp,
        colorMode: commonSchemas.stringProp,
        colorCurve: commonSchemas.arrayOfObjects,
        materialPath: commonSchemas.materialPath,
        meshPath: commonSchemas.meshPath,
        sortMode: commonSchemas.stringProp,
        alignment: commonSchemas.stringProp,
        facingMode: commonSchemas.stringProp,
        ribbonWidth: commonSchemas.numberProp,
        ribbonTwist: commonSchemas.numberProp,
        ribbonFacingMode: commonSchemas.stringProp,
        tessellationFactor: commonSchemas.numberProp,
        lightRadius: commonSchemas.numberProp,
        lightIntensity: commonSchemas.numberProp,
        lightColor: commonSchemas.objectProp,
        volumetricScattering: commonSchemas.numberProp,
        lightExponent: commonSchemas.numberProp,
        affectsTranslucency: commonSchemas.booleanProp,
        collisionMode: commonSchemas.stringProp,
        restitution: commonSchemas.numberProp,
        friction: commonSchemas.numberProp,
        radiusScale: commonSchemas.numberProp,
        dieOnCollision: commonSchemas.booleanProp,
        killCondition: commonSchemas.stringProp,
        killBox: commonSchemas.objectProp,
        invertKillZone: commonSchemas.booleanProp,
        cameraOffset: commonSchemas.numberProp,
        cameraOffsetMode: commonSchemas.stringProp,
        parameterValue: commonSchemas.value,
        sourceBinding: commonSchemas.stringProp,
        skeletalMeshPath: commonSchemas.skeletalMeshPath,
        staticMeshPath: commonSchemas.meshAssetPath,
        useWholeSkeletonOrBones: commonSchemas.stringProp,
        specificBones: commonSchemas.arrayOfStrings,
        samplingMode: commonSchemas.stringProp,
        eventName: commonSchemas.eventName,
        eventPayload: commonSchemas.arrayOfObjects,
        spawnOnEvent: commonSchemas.booleanProp,
        eventSpawnCount: commonSchemas.numberProp,
        gpuEnabled: commonSchemas.booleanProp,
        fixedBoundsEnabled: commonSchemas.booleanProp,
        deterministicEnabled: commonSchemas.booleanProp,
        stageName: commonSchemas.stageName,
        stageIterationSource: commonSchemas.stringProp
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.assetPath,
        emitterName: commonSchemas.stringProp,
        moduleName: commonSchemas.stringProp,
        niagaraInfo: commonSchemas.objectProp,
        validationResult: commonSchemas.objectProp
      }
    }
  },
  {
    name: 'build_environment',
    category: 'world',
    description: 'Create/sculpt landscapes, paint foliage, and generate procedural terrain/biomes.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_landscape', 'sculpt', 'sculpt_landscape', 'add_foliage', 'paint_foliage',
            'create_procedural_terrain', 'create_procedural_foliage', 'add_foliage_instances',
            'get_foliage_instances', 'remove_foliage', 'paint_landscape', 'paint_landscape_layer',
            'modify_heightmap', 'set_landscape_material', 'create_landscape_grass_type',
            'generate_lods', 'bake_lightmap', 'export_snapshot', 'import_snapshot', 'delete'
          ],
          description: 'Action'
        },
        name: commonSchemas.name,
        landscapeName: commonSchemas.stringProp,
        heightData: commonSchemas.arrayOfNumbers,
        minX: commonSchemas.numberProp,
        minY: commonSchemas.numberProp,
        maxX: commonSchemas.numberProp,
        maxY: commonSchemas.numberProp,
        updateNormals: commonSchemas.booleanProp,
        location: commonSchemas.location,
        rotation: commonSchemas.rotation,
        scale: commonSchemas.scale,
        sizeX: commonSchemas.numberProp,
        sizeY: commonSchemas.numberProp,
        sectionSize: commonSchemas.numberProp,
        sectionsPerComponent: commonSchemas.numberProp,
        componentCount: commonSchemas.vector2,
        materialPath: commonSchemas.materialPath,
        tool: commonSchemas.stringProp,
        radius: commonSchemas.numberProp,
        strength: commonSchemas.numberProp,
        falloff: commonSchemas.numberProp,
        brushSize: commonSchemas.numberProp,
        layerName: commonSchemas.stringProp,
        eraseMode: commonSchemas.booleanProp,
        foliageType: commonSchemas.stringProp,
        foliageTypePath: commonSchemas.assetPath,
        meshPath: commonSchemas.meshPath,
        density: commonSchemas.numberProp,
        minScale: commonSchemas.numberProp,
        maxScale: commonSchemas.numberProp,
        cullDistance: commonSchemas.numberProp,
        alignToNormal: commonSchemas.booleanProp,
        randomYaw: commonSchemas.booleanProp,
        locations: { type: 'array', items: commonSchemas.location },
        transforms: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              location: commonSchemas.location,
              rotation: commonSchemas.rotation,
              scale: commonSchemas.scale
            }
          }
        },
        position: commonSchemas.location,
        bounds: commonSchemas.objectProp,
        volumeName: commonSchemas.stringProp,
        seed: commonSchemas.numberProp,
        foliageTypes: commonSchemas.arrayOfObjects,
        path: commonSchemas.directoryPath,
        filename: commonSchemas.stringProp,
        assetPaths: commonSchemas.arrayOfStrings
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase
      }
    }
  },
  {
    name: 'system_control',
    category: 'core',
    description: 'Run profiling, set quality/CVars, execute console commands, run UBT, and manage widgets.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'profile', 'show_fps', 'set_quality', 'screenshot', 'set_resolution', 'set_fullscreen', 'execute_command', 'console_command',
            'run_ubt', 'run_tests', 'subscribe', 'unsubscribe', 'spawn_category', 'start_session', 'lumen_update_scene',
            'play_sound', 'create_widget', 'show_widget', 'add_widget_child',
            'set_cvar', 'get_project_settings', 'validate_assets',
            'set_project_setting'
          ],
          description: 'Action'
        },
        profileType: commonSchemas.stringProp,
        category: commonSchemas.stringProp,
        level: commonSchemas.numberProp,
        enabled: commonSchemas.enabled,
        resolution: commonSchemas.resolution,
        command: commonSchemas.stringProp,
        target: commonSchemas.stringProp,
        platform: commonSchemas.stringProp,
        configuration: commonSchemas.stringProp,
        arguments: commonSchemas.stringProp,
        filter: commonSchemas.stringProp,
        channels: commonSchemas.stringProp,
        widgetPath: commonSchemas.widgetPath,
        childClass: commonSchemas.stringProp,
        parentName: commonSchemas.stringProp,
        section: commonSchemas.stringProp,
        key: commonSchemas.stringProp,
        value: commonSchemas.stringProp,
        configName: commonSchemas.stringProp
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        output: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_sequence',
    category: 'utility',
    description: 'Edit Level Sequences: add tracks, bind actors, set keyframes, control playback, and record camera.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create', 'open', 'add_camera', 'add_actor', 'add_actors', 'remove_actors',
            'get_bindings', 'play', 'pause', 'stop', 'set_playback_speed', 'add_keyframe',
            'get_properties', 'set_properties', 'duplicate', 'rename', 'delete', 'list', 'get_metadata', 'set_metadata',
            'add_spawnable_from_class', 'add_track', 'add_section', 'set_display_rate', 'set_tick_resolution',
            'set_work_range', 'set_view_range', 'set_track_muted', 'set_track_solo', 'set_track_locked',
            'list_tracks', 'remove_track', 'list_track_types'
          ],
          description: 'Action'
        },
        name: commonSchemas.name,
        path: commonSchemas.assetPath,
        actorName: commonSchemas.actorName,
        actorNames: commonSchemas.arrayOfStrings,
        frame: commonSchemas.numberProp,
        value: commonSchemas.objectProp,
        property: commonSchemas.propertyName,
        destinationPath: commonSchemas.destinationPath,
        newName: commonSchemas.newName,
        overwrite: commonSchemas.overwrite,
        speed: commonSchemas.numberProp,
        startTime: commonSchemas.numberProp,
        loopMode: commonSchemas.stringProp,
        className: commonSchemas.stringProp,
        spawnable: commonSchemas.booleanProp,
        trackType: commonSchemas.stringProp,
        trackName: commonSchemas.stringProp,
        muted: commonSchemas.booleanProp,
        solo: commonSchemas.booleanProp,
        locked: commonSchemas.booleanProp,
        assetPath: commonSchemas.assetPath,
        startFrame: commonSchemas.numberProp,
        endFrame: commonSchemas.numberProp,
        frameRate: commonSchemas.stringProp,
        resolution: commonSchemas.stringProp,
        start: commonSchemas.numberProp,
        end: commonSchemas.numberProp,
        lengthInFrames: commonSchemas.numberProp,
        playbackStart: commonSchemas.numberProp,
        playbackEnd: commonSchemas.numberProp,
        metadata: commonSchemas.objectProp
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase
      }
    }
  },
  {
    name: 'manage_input',
    category: 'utility',
    description: 'Create Input Actions and Mapping Contexts. Add key/gamepad bindings with modifiers and triggers.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_input_action',
            'create_input_mapping_context',
            'add_mapping',
            'remove_mapping'
          ],
          description: 'Action to perform'
        },
        name: commonSchemas.name,
        path: commonSchemas.directoryPath,
        contextPath: commonSchemas.assetPath,
        actionPath: commonSchemas.assetPath,
        key: commonSchemas.stringProp
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.assetPath
      }
    }
  },
  {
    name: 'inspect',
    category: 'core',
    description: 'Inspect any UObject: read/write properties, list components, export snapshots, and query class info.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'inspect_object', 'set_property', 'get_property', 'get_components', 'inspect_class', 'list_objects',
            'get_component_property', 'set_component_property', 'get_metadata', 'add_tag', 'find_by_tag',
            'create_snapshot', 'restore_snapshot', 'export', 'delete_object', 'find_by_class', 'get_bounding_box'
          ],
          description: 'Action'
        },
        objectPath: commonSchemas.assetPath,
        propertyName: commonSchemas.propertyName,
        propertyPath: commonSchemas.stringProp,
        value: commonSchemas.value,
        actorName: commonSchemas.actorName,
        name: commonSchemas.name,
        componentName: commonSchemas.componentName,
        className: commonSchemas.stringProp,
        classPath: commonSchemas.assetPath,
        tag: commonSchemas.tagName,
        filter: commonSchemas.stringProp,
        snapshotName: commonSchemas.stringProp,
        destinationPath: commonSchemas.destinationPath,
        outputPath: commonSchemas.outputPath,
        format: commonSchemas.stringProp
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        value: commonSchemas.value
      }
    }
  },
  {
    name: 'manage_audio',
    category: 'utility',
    description: 'Play/stop sounds, add audio components, configure mixes, attenuation, spatial audio, and author Sound Cues/MetaSounds.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            // Runtime audio control
            'create_sound_cue', 'play_sound_at_location', 'play_sound_2d', 'create_audio_component',
            'create_sound_mix', 'push_sound_mix', 'pop_sound_mix',
            'set_sound_mix_class_override', 'clear_sound_mix_class_override', 'set_base_sound_mix',
            'prime_sound', 'play_sound_attached', 'spawn_sound_at_location',
            'fade_sound_in', 'fade_sound_out', 'create_ambient_sound',
            'create_sound_class', 'set_sound_attenuation', 'create_reverb_zone',
            'enable_audio_analysis', 'fade_sound', 'set_doppler_effect', 'set_audio_occlusion',
            // Sound Cue authoring (merged from manage_audio_authoring)
            'add_cue_node', 'connect_cue_nodes', 'set_cue_attenuation', 'set_cue_concurrency',
            // MetaSound authoring
            'create_metasound', 'add_metasound_node', 'connect_metasound_nodes',
            'add_metasound_input', 'add_metasound_output', 'set_metasound_default',
            // Sound class/mix authoring
            'set_class_properties', 'set_class_parent', 'add_mix_modifier', 'configure_mix_eq',
            // Attenuation authoring
            'create_attenuation_settings', 'configure_distance_attenuation',
            'configure_spatialization', 'configure_occlusion', 'configure_reverb_send',
            // Dialogue system
            'create_dialogue_voice', 'create_dialogue_wave', 'set_dialogue_context',
            // Effects
            'create_reverb_effect', 'create_source_effect_chain', 'add_source_effect', 'create_submix_effect',
            // Utility
            'get_audio_info'
          ],
          description: 'Action'
        },
        name: commonSchemas.name,
        soundPath: commonSchemas.soundPath,
        location: commonSchemas.location,
        rotation: commonSchemas.rotation,
        volume: commonSchemas.numberProp,
        pitch: commonSchemas.numberProp,
        startTime: commonSchemas.numberProp,
        attenuationPath: commonSchemas.assetPath,
        concurrencyPath: commonSchemas.assetPath,
        mixName: commonSchemas.stringProp,
        soundClassName: commonSchemas.stringProp,
        fadeInTime: commonSchemas.numberProp,
        fadeOutTime: commonSchemas.numberProp,
        fadeTime: commonSchemas.numberProp,
        targetVolume: commonSchemas.numberProp,
        attachPointName: commonSchemas.socketName,
        actorName: commonSchemas.actorName,
        componentName: commonSchemas.componentName,
        parentClass: commonSchemas.stringProp,
        properties: commonSchemas.objectProp,
        innerRadius: commonSchemas.numberProp,
        falloffDistance: commonSchemas.numberProp,
        attenuationShape: commonSchemas.stringProp,
        falloffMode: commonSchemas.stringProp,
        reverbEffect: commonSchemas.stringProp,
        size: commonSchemas.scale,
        fftSize: commonSchemas.numberProp,
        outputType: commonSchemas.stringProp,
        soundName: commonSchemas.stringProp,
        fadeType: commonSchemas.stringProp,
        scale: commonSchemas.numberProp,
        lowPassFilterFrequency: commonSchemas.numberProp,
        volumeAttenuation: commonSchemas.numberProp,
        enabled: commonSchemas.enabled,
        // Authoring properties (merged from manage_audio_authoring)
        path: commonSchemas.directoryPathForCreation,
        assetPath: commonSchemas.assetPath,
        save: commonSchemas.save,
        wavePath: commonSchemas.wavePath,
        nodeType: commonSchemas.stringProp,
        nodeId: commonSchemas.nodeId,
        sourceNodeId: commonSchemas.sourceNodeId,
        targetNodeId: commonSchemas.targetNodeId,
        outputPin: commonSchemas.numberProp,
        inputPin: commonSchemas.numberProp,
        looping: commonSchemas.looping,
        x: commonSchemas.numberProp,
        y: commonSchemas.numberProp,
        metasoundType: commonSchemas.stringProp,
        inputName: commonSchemas.inputName,
        inputType: commonSchemas.stringProp,
        outputName: commonSchemas.outputName,
        sourceNode: commonSchemas.sourceNode,
        sourcePin: commonSchemas.sourcePin,
        targetNode: commonSchemas.targetNode,
        targetPin: commonSchemas.targetPin,
        defaultValue: commonSchemas.value,
        metasoundNodeType: commonSchemas.stringProp,
        soundClassPath: commonSchemas.soundClassPath,
        parentClassPath: commonSchemas.parentClassPath
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.assetPath,
        nodeId: commonSchemas.nodeId
      }
    }
  },
  {
    name: 'manage_behavior_tree',
    category: 'utility',
    description: 'Create Behavior Trees, add task/decorator/service nodes, and configure node properties.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: ['create', 'add_node', 'connect_nodes', 'remove_node', 'break_connections', 'set_node_properties'],
          description: 'Action'
        },
        name: commonSchemas.name,
        savePath: commonSchemas.savePath,
        assetPath: commonSchemas.assetPath,
        nodeType: commonSchemas.stringProp,
        nodeId: commonSchemas.nodeId,
        parentNodeId: commonSchemas.nodeId,
        childNodeId: commonSchemas.nodeId,
        x: commonSchemas.numberProp,
        y: commonSchemas.numberProp,
        comment: commonSchemas.stringProp,
        properties: commonSchemas.objectProp
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputWithNodeId
      }
    }
  },
  // [MERGED] manage_blueprint_graph actions now in manage_blueprint (Phase 53: Strategic Tool Merging)
  {
    name: 'manage_lighting',
    category: 'world',
    description: 'Spawn lights (point, spot, rect, sky), configure GI, shadows, volumetric fog, and build lighting.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'spawn_light', 'create_light', 'spawn_sky_light', 'create_sky_light', 'ensure_single_sky_light',
            'create_lightmass_volume', 'create_lighting_enabled_level', 'create_dynamic_light',
            'setup_global_illumination', 'configure_shadows', 'set_exposure', 'set_ambient_occlusion', 'setup_volumetric_fog',
            'build_lighting', 'list_light_types'
          ],
          description: 'Action'
        },
        name: commonSchemas.name,
        location: commonSchemas.location,
        rotation: commonSchemas.rotation,
        lightType: { type: 'string', enum: ['Directional', 'Point', 'Spot', 'Rect'] },
        intensity: commonSchemas.numberProp,
        color: commonSchemas.color,
        castShadows: commonSchemas.booleanProp,
        useAsAtmosphereSunLight: { type: 'boolean', description: 'For Directional Lights, use as Atmosphere Sun Light.' },
        temperature: commonSchemas.numberProp,
        radius: commonSchemas.numberProp,
        falloffExponent: commonSchemas.numberProp,
        innerCone: commonSchemas.numberProp,
        outerCone: commonSchemas.numberProp,
        width: commonSchemas.numberProp,
        height: commonSchemas.numberProp,
        sourceType: { type: 'string', enum: ['CapturedScene', 'SpecifiedCubemap'] },
        cubemapPath: commonSchemas.texturePath,
        recapture: commonSchemas.booleanProp,
        method: { type: 'string', enum: ['Lightmass', 'LumenGI', 'ScreenSpace', 'None'] },
        quality: commonSchemas.stringProp,
        indirectLightingIntensity: commonSchemas.numberProp,
        bounces: commonSchemas.numberProp,
        shadowQuality: commonSchemas.stringProp,
        cascadedShadows: commonSchemas.booleanProp,
        shadowDistance: commonSchemas.numberProp,
        contactShadows: commonSchemas.booleanProp,
        rayTracedShadows: commonSchemas.booleanProp,
        compensationValue: commonSchemas.numberProp,
        minBrightness: commonSchemas.numberProp,
        maxBrightness: commonSchemas.numberProp,
        enabled: commonSchemas.enabled,
        density: commonSchemas.numberProp,
        scatteringIntensity: commonSchemas.numberProp,
        fogHeight: commonSchemas.numberProp,
        buildOnlySelected: commonSchemas.booleanProp,
        buildReflectionCaptures: commonSchemas.booleanProp,
        levelName: commonSchemas.stringProp,
        copyActors: commonSchemas.booleanProp,
        useTemplate: commonSchemas.booleanProp,
        size: commonSchemas.scale
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        actorName: commonSchemas.actorName
      }
    }
  },
  {
    name: 'manage_performance',
    category: 'utility',
    description: 'Run profiling/benchmarks, configure scalability, LOD, Nanite, and optimization settings.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'start_profiling', 'stop_profiling', 'run_benchmark', 'show_fps', 'show_stats', 'generate_memory_report',
            'set_scalability', 'set_resolution_scale', 'set_vsync', 'set_frame_rate_limit', 'enable_gpu_timing',
            'configure_texture_streaming', 'configure_lod', 'apply_baseline_settings', 'optimize_draw_calls', 'merge_actors',
            'configure_occlusion_culling', 'optimize_shaders', 'configure_nanite', 'configure_world_partition'
          ],
          description: 'Action'
        },
        type: { type: 'string', enum: ['CPU', 'GPU', 'Memory', 'RenderThread', 'GameThread', 'All'] },
        duration: commonSchemas.numberProp,
        outputPath: commonSchemas.outputPath,
        detailed: commonSchemas.booleanProp,
        category: commonSchemas.stringProp,
        level: commonSchemas.numberProp,
        scale: commonSchemas.numberProp,
        enabled: commonSchemas.enabled,
        maxFPS: commonSchemas.numberProp,
        verbose: commonSchemas.booleanProp,
        poolSize: commonSchemas.numberProp,
        boostPlayerLocation: commonSchemas.booleanProp,
        forceLOD: commonSchemas.numberProp,
        lodBias: commonSchemas.numberProp,
        distanceScale: commonSchemas.numberProp,
        skeletalBias: commonSchemas.numberProp,
        hzb: commonSchemas.booleanProp,
        enableInstancing: commonSchemas.booleanProp,
        enableBatching: commonSchemas.booleanProp,
        mergeActors: commonSchemas.booleanProp,
        actors: commonSchemas.arrayOfStrings,
        freezeRendering: commonSchemas.booleanProp,
        compileOnDemand: commonSchemas.booleanProp,
        cacheShaders: commonSchemas.booleanProp,
        reducePermutations: commonSchemas.booleanProp,
        maxPixelsPerEdge: commonSchemas.numberProp,
        streamingPoolSize: commonSchemas.numberProp,
        streamingDistance: commonSchemas.numberProp,
        cellSize: commonSchemas.numberProp
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        params: commonSchemas.objectProp
      }
    }
  },
  {
    name: 'manage_geometry',
    category: 'world',
    description: 'Create procedural meshes using Geometry Script: booleans, deformers, UVs, collision, and LOD generation.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_box', 'create_sphere', 'create_cylinder', 'create_cone', 'create_capsule',
            'create_torus', 'create_plane', 'create_disc',
            'create_stairs', 'create_spiral_stairs', 'create_ring',
            'create_arch', 'create_pipe', 'create_ramp',
            'boolean_union', 'boolean_subtract', 'boolean_intersection',
            'boolean_trim', 'self_union',
            'extrude', 'inset', 'outset', 'bevel', 'offset_faces', 'shell', 'revolve', 'chamfer',
            'extrude_along_spline', 'bridge', 'loft', 'sweep',
            'duplicate_along_spline', 'loop_cut', 'edge_split', 'quadrangulate',
            'bend', 'twist', 'taper', 'noise_deform', 'smooth', 'relax',
            'stretch', 'spherify', 'cylindrify',
            'triangulate', 'poke',
            'mirror', 'array_linear', 'array_radial',
            'simplify_mesh', 'subdivide', 'remesh_uniform', 'merge_vertices', 'remesh_voxel',
            'weld_vertices', 'fill_holes', 'remove_degenerates',
            'auto_uv', 'project_uv', 'transform_uvs', 'unwrap_uv', 'pack_uv_islands',
            'recalculate_normals', 'flip_normals', 'recompute_tangents',
            'generate_collision', 'generate_complex_collision', 'simplify_collision',
            'generate_lods', 'set_lod_settings', 'set_lod_screen_sizes', 'convert_to_nanite',
            'convert_to_static_mesh',
            'get_mesh_info'
          ],
          description: 'Geometry action to perform'
        },
        meshPath: commonSchemas.meshPath,
        targetMeshPath: { type: 'string', description: 'Path to second mesh for boolean operations.' },
        outputPath: commonSchemas.outputPath,
        actorName: commonSchemas.actorName,
        width: commonSchemas.width,
        height: commonSchemas.height,
        depth: commonSchemas.depth,
        radius: commonSchemas.radius,
        innerRadius: { type: 'number', description: 'Inner radius for torus.' },
        numSides: { type: 'number', description: 'Number of sides for cylinder, cone, etc.' },
        numRings: { type: 'number', description: 'Number of rings for sphere, torus.' },
        numSteps: { type: 'number', description: 'Number of steps for stairs.' },
        stepWidth: { type: 'number', description: 'Width of each stair step.' },
        stepHeight: { type: 'number', description: 'Height of each stair step.' },
        stepDepth: { type: 'number', description: 'Depth of each stair step.' },
        numTurns: { type: 'number', description: 'Number of turns for spiral.' },
        widthSegments: { type: 'number', description: 'Segments along width.' },
        heightSegments: { type: 'number', description: 'Segments along height.' },
        depthSegments: { type: 'number', description: 'Segments along depth.' },
        radialSegments: { type: 'number', description: 'Radial segments for circular shapes.' },
        location: commonSchemas.location,
        rotation: commonSchemas.rotation,
        scale: commonSchemas.scale,
        distance: commonSchemas.distance,
        amount: { type: 'number', description: 'Generic amount for operations (bevel size, inset distance, etc.).' },
        segments: { type: 'number', description: 'Number of segments for bevel, subdivide.' },
        angle: commonSchemas.angle,
        axis: { type: 'string', enum: ['X', 'Y', 'Z'], description: 'Axis for deformation operations.' },
        strength: commonSchemas.strength,
        iterations: { type: 'number', description: 'Number of iterations for smooth, remesh.' },
        targetTriangleCount: { type: 'number', description: 'Target triangle count for simplification.' },
        targetEdgeLength: { type: 'number', description: 'Target edge length for remeshing.' },
        weldDistance: { type: 'number', description: 'Distance threshold for vertex welding.' },
        faceIndices: { type: 'array', items: commonSchemas.numberProp, description: 'Array of face indices.' },
        edgeIndices: { type: 'array', items: commonSchemas.numberProp, description: 'Array of edge indices.' },
        vertexIndices: { type: 'array', items: commonSchemas.numberProp, description: 'Array of vertex indices.' },
        selectionBox: { type: 'object', properties: { min: commonSchemas.objectProp, max: commonSchemas.objectProp }, description: 'Bounding box for selection.' },
        uvChannel: { type: 'number', description: 'UV channel index (0-7).' },
        uvScale: { type: 'object', properties: { u: commonSchemas.numberProp, v: commonSchemas.numberProp }, description: 'UV scale.' },
        uvOffset: { type: 'object', properties: { u: commonSchemas.numberProp, v: commonSchemas.numberProp }, description: 'UV offset.' },
        projectionDirection: { type: 'string', enum: ['X', 'Y', 'Z', 'Auto'], description: 'Projection direction for UV.' },
        hardEdgeAngle: { type: 'number', description: 'Angle threshold for hard edges (degrees).' },
        computeWeightedNormals: { type: 'boolean', description: 'Use area-weighted normals.' },
        smoothingGroupId: { type: 'number', description: 'Smoothing group ID.' },
        collisionType: { type: 'string', enum: ['Default', 'Simple', 'Complex', 'UseComplexAsSimple', 'UseSimpleAsComplex'], description: 'Collision complexity type.' },
        hullCount: { type: 'number', description: 'Number of convex hulls for decomposition.' },
        hullPrecision: { type: 'number', description: 'Precision for convex hull generation (0-1).' },
        maxVerticesPerHull: { type: 'number', description: 'Maximum vertices per convex hull.' },
        lodCount: { type: 'number', description: 'Number of LOD levels to generate.' },
        lodIndex: { type: 'number', description: 'Specific LOD index to configure.' },
        reductionPercent: { type: 'number', description: 'Percent of triangles to reduce per LOD.' },
        screenSize: { type: 'number', description: 'Screen size threshold for LOD switching.' },
        screenSizes: { type: 'array', items: commonSchemas.numberProp, description: 'Array of screen sizes for each LOD.' },
        preserveBorders: { type: 'boolean', description: 'Preserve mesh borders during LOD generation.' },
        preserveUVs: { type: 'boolean', description: 'Preserve UV seams during LOD generation.' },
        exportFormat: { type: 'string', enum: ['FBX', 'OBJ', 'glTF', 'USD'], description: 'Export file format.' },
        exportPath: commonSchemas.exportPath,
        includeNormals: { type: 'boolean', description: 'Include normals in export.' },
        includeUVs: { type: 'boolean', description: 'Include UVs in export.' },
        includeTangents: { type: 'boolean', description: 'Include tangents in export.' },
        createAsset: { type: 'boolean', description: 'Create as persistent asset.' },
        overwrite: commonSchemas.overwrite,
        save: commonSchemas.save,
        enableNanite: { type: 'boolean', description: 'Enable Nanite for the output mesh.' }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        meshPath: commonSchemas.meshPath,
        actorName: commonSchemas.actorName,
        meshInfo: {
          type: 'object',
          properties: {
            vertexCount: commonSchemas.numberProp,
            triangleCount: commonSchemas.numberProp,
            uvChannels: commonSchemas.numberProp,
            hasNormals: commonSchemas.booleanProp,
            hasTangents: commonSchemas.booleanProp,
            boundingBox: commonSchemas.objectProp,
            lodCount: commonSchemas.numberProp
          }
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_skeleton',
    category: 'authoring',
    description: 'Edit skeletal meshes: add sockets, configure physics assets, set skin weights, and create morph targets.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_skeleton', 'add_bone', 'remove_bone', 'rename_bone',
            'set_bone_transform', 'set_bone_parent',
            'create_virtual_bone',
            'create_socket', 'configure_socket',
            'auto_skin_weights', 'set_vertex_weights',
            'normalize_weights', 'prune_weights',
            'copy_weights', 'mirror_weights',
            'create_physics_asset',
            'add_physics_body', 'configure_physics_body',
            'add_physics_constraint', 'configure_constraint_limits',
            'bind_cloth_to_skeletal_mesh', 'assign_cloth_asset_to_mesh',
            'create_morph_target', 'set_morph_target_deltas', 'import_morph_targets',
            'get_skeleton_info', 'list_bones', 'list_sockets', 'list_physics_bodies'
          ],
          description: 'Skeleton action to perform'
        },
        skeletonPath: commonSchemas.skeletonPath,
        skeletalMeshPath: commonSchemas.skeletalMeshPath,
        physicsAssetPath: commonSchemas.physicsAssetPath,
        morphTargetPath: { type: 'string', description: 'Path to morph target or FBX file for import.' },
        clothAssetPath: commonSchemas.clothAssetPath,
        outputPath: commonSchemas.outputPath,
        boneName: commonSchemas.boneName,
        newBoneName: commonSchemas.newName,
        parentBoneName: commonSchemas.parentBoneName,
        sourceBoneName: commonSchemas.sourceBoneName,
        targetBoneName: commonSchemas.targetBoneName,
        boneIndex: { type: 'number', description: 'Bone index for operations.' },
        location: commonSchemas.location,
        rotation: commonSchemas.rotation,
        scale: commonSchemas.scale,
        socketName: commonSchemas.socketName,
        attachBoneName: commonSchemas.attachBoneName,
        relativeLocation: commonSchemas.location,
        relativeRotation: commonSchemas.rotation,
        relativeScale: commonSchemas.scale,
        vertexIndex: { type: 'number', description: 'Vertex index for weight operations.' },
        vertexIndices: { type: 'array', items: commonSchemas.numberProp, description: 'Array of vertex indices.' },
        weights: { type: 'array', items: commonSchemas.objectProp, description: 'Array of {boneIndex, weight} pairs.' },
        threshold: { type: 'number', description: 'Weight threshold for pruning (0-1).' },
        mirrorAxis: { type: 'string', enum: ['X', 'Y', 'Z'], description: 'Axis for weight mirroring.' },
        mirrorTable: { type: 'object', description: 'Bone name mapping for mirroring.' },
        bodyType: { type: 'string', enum: ['Capsule', 'Sphere', 'Box', 'Convex', 'Sphyl'], description: 'Physics body shape type.' },
        bodyName: commonSchemas.bodyName,
        mass: commonSchemas.mass,
        linearDamping: { type: 'number', description: 'Linear damping factor.' },
        angularDamping: { type: 'number', description: 'Angular damping factor.' },
        collisionEnabled: { type: 'boolean', description: 'Enable collision for this body.' },
        simulatePhysics: { type: 'boolean', description: 'Enable physics simulation.' },
        constraintName: commonSchemas.constraintName,
        bodyA: commonSchemas.bodyA,
        bodyB: commonSchemas.bodyB,
        limits: {
          type: 'object',
          properties: {
            swing1LimitAngle: { type: 'number', description: 'Swing 1 limit in degrees.' },
            swing2LimitAngle: { type: 'number', description: 'Swing 2 limit in degrees.' },
            twistLimitAngle: { type: 'number', description: 'Twist limit in degrees.' },
            swing1Motion: { type: 'string', enum: ['Free', 'Limited', 'Locked'] },
            swing2Motion: { type: 'string', enum: ['Free', 'Limited', 'Locked'] },
            twistMotion: { type: 'string', enum: ['Free', 'Limited', 'Locked'] }
          },
          description: 'Constraint angular limits.'
        },
        morphTargetName: commonSchemas.morphTargetName,
        deltas: { type: 'array', items: commonSchemas.objectProp, description: 'Array of {vertexIndex, delta} for morph target.' },
        paintValue: { type: 'number', description: 'Cloth weight paint value (0-1).' },
        save: commonSchemas.save,
        overwrite: commonSchemas.overwrite
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        skeletonPath: commonSchemas.skeletonPath,
        physicsAssetPath: commonSchemas.assetPath,
        socketName: commonSchemas.socketName,
        boneInfo: {
          type: 'object',
          properties: {
            name: commonSchemas.stringProp,
            index: commonSchemas.numberProp,
            parentName: commonSchemas.stringProp,
            parentIndex: commonSchemas.numberProp
          }
        },
        bones: commonSchemas.arrayOfObjects,
        sockets: commonSchemas.arrayOfObjects,
        physicsBodies: commonSchemas.arrayOfObjects,
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_material_authoring',
    category: 'authoring',
    description: 'Create materials with expressions, parameters, functions, instances, and landscape blend layers.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_material', 'set_blend_mode', 'set_shading_model', 'set_material_domain',
            'add_texture_sample', 'add_texture_coordinate', 'add_scalar_parameter', 'add_vector_parameter',
            'add_static_switch_parameter', 'add_math_node', 'add_world_position', 'add_vertex_normal',
            'add_pixel_depth', 'add_fresnel', 'add_reflection_vector', 'add_panner', 'add_rotator',
            'add_noise', 'add_voronoi', 'add_if', 'add_switch', 'add_custom_expression',
            'connect_nodes', 'disconnect_nodes',
            'create_material_function', 'add_function_input', 'add_function_output', 'use_material_function',
            'create_material_instance', 'set_scalar_parameter_value', 'set_vector_parameter_value', 'set_texture_parameter_value',
            'create_landscape_material', 'create_decal_material', 'create_post_process_material',
            'add_landscape_layer', 'configure_layer_blend',
            'compile_material', 'get_material_info'
          ],
          description: 'Material authoring action to perform'
        },
        assetPath: commonSchemas.assetPath,
        name: commonSchemas.name,
        path: commonSchemas.directoryPathForCreation,
        materialDomain: { type: 'string', enum: ['Surface', 'DeferredDecal', 'LightFunction', 'Volume', 'PostProcess', 'UI'], description: 'Material domain type.' },
        blendMode: { type: 'string', enum: ['Opaque', 'Masked', 'Translucent', 'Additive', 'Modulate', 'AlphaComposite', 'AlphaHoldout'], description: 'Blend mode.' },
        shadingModel: { type: 'string', enum: ['DefaultLit', 'Unlit', 'Subsurface', 'SubsurfaceProfile', 'PreintegratedSkin', 'ClearCoat', 'Hair', 'Cloth', 'Eye', 'TwoSidedFoliage', 'ThinTranslucent'], description: 'Shading model.' },
        twoSided: { type: 'boolean', description: 'Enable two-sided rendering.' },
        x: commonSchemas.nodeX,
        y: commonSchemas.nodeY,
        texturePath: commonSchemas.texturePath,
        samplerType: { type: 'string', enum: ['Color', 'LinearColor', 'Normal', 'Masks', 'Alpha', 'VirtualColor', 'VirtualNormal'], description: 'Texture sampler type.' },
        coordinateIndex: { type: 'number', description: 'UV channel index (0-7).' },
        uTiling: { type: 'number', description: 'U tiling factor.' },
        vTiling: { type: 'number', description: 'V tiling factor.' },
        parameterName: commonSchemas.parameterName,
        defaultValue: { description: 'Default value for parameter (number for scalar, object for vector, bool for switch).' },
        group: commonSchemas.group,
        value: { description: 'Value to set (number, vector object, or texture path).' },
        operation: { type: 'string', enum: ['Add', 'Subtract', 'Multiply', 'Divide', 'Lerp', 'Clamp', 'Power', 'SquareRoot', 'Abs', 'Floor', 'Ceil', 'Frac', 'Sine', 'Cosine', 'Saturate', 'OneMinus', 'Min', 'Max', 'Dot', 'Cross', 'Normalize', 'Append'], description: 'Math operation type.' },
        constA: { type: 'number', description: 'Constant A input value.' },
        constB: { type: 'number', description: 'Constant B input value.' },
        code: commonSchemas.code,
        outputType: { type: 'string', enum: ['Float1', 'Float2', 'Float3', 'Float4', 'MaterialAttributes'], description: 'Output type of custom expression.' },
        description: { type: 'string', description: 'Description for custom expression or function.' },
        sourceNodeId: { type: 'string', description: 'Source node ID for connection.' },
        sourcePin: { type: 'string', description: 'Source pin name (output).' },
        targetNodeId: { type: 'string', description: 'Target node ID for connection.' },
        targetPin: { type: 'string', description: 'Target pin name (input).' },
        nodeId: commonSchemas.nodeId,
        pinName: commonSchemas.pinName,
        functionPath: commonSchemas.functionPath,
        exposeToLibrary: { type: 'boolean', description: 'Expose function to material library.' },
        inputName: commonSchemas.inputName,
        inputType: { type: 'string', enum: ['Float1', 'Float2', 'Float3', 'Float4', 'Texture2D', 'TextureCube', 'Bool', 'MaterialAttributes'], description: 'Type of function input/output.' },
        parentMaterial: { type: 'string', description: 'Path to parent material for instances.' },
        layerName: commonSchemas.layerName,
        blendType: { type: 'string', enum: ['LB_WeightBlend', 'LB_AlphaBlend', 'LB_HeightBlend'], description: 'Landscape layer blend type.' },
        layers: { type: 'array', items: commonSchemas.objectProp, description: 'Array of layer configurations for layer blend.' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.assetPath,
        nodeId: commonSchemas.nodeId,
        materialInfo: {
          type: 'object',
          properties: {
            domain: commonSchemas.stringProp,
            blendMode: commonSchemas.stringProp,
            shadingModel: commonSchemas.stringProp,
            twoSided: commonSchemas.booleanProp,
            nodeCount: commonSchemas.numberProp,
            parameters: commonSchemas.arrayOfObjects
          }
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_texture',
    category: 'authoring',
    description: 'Create procedural textures, process images, bake normal/AO maps, and set compression settings.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_noise_texture', 'create_gradient_texture', 'create_pattern_texture',
            'create_normal_from_height', 'create_ao_from_mesh',
            'resize_texture', 'adjust_levels', 'adjust_curves', 'blur', 'sharpen',
            'invert', 'desaturate', 'channel_pack', 'channel_extract', 'combine_textures',
            'set_compression_settings', 'set_texture_group', 'set_lod_bias',
            'configure_virtual_texture', 'set_streaming_priority',
            'get_texture_info'
          ],
          description: 'Texture action to perform'
        },
        assetPath: commonSchemas.assetPath,
        name: commonSchemas.name,
        path: commonSchemas.directoryPathForCreation,
        width: commonSchemas.width,
        height: commonSchemas.height,
        newWidth: { type: 'number', description: 'New width for resize operation.' },
        newHeight: { type: 'number', description: 'New height for resize operation.' },
        noiseType: { type: 'string', enum: ['Perlin', 'Simplex', 'Worley', 'Voronoi'], description: 'Type of noise to generate.' },
        scale: { type: 'number', description: 'Noise scale/frequency.' },
        octaves: { type: 'number', description: 'Number of noise octaves for FBM.' },
        persistence: { type: 'number', description: 'Amplitude falloff per octave.' },
        lacunarity: { type: 'number', description: 'Frequency multiplier per octave.' },
        seed: { type: 'number', description: 'Random seed for procedural generation.' },
        seamless: { type: 'boolean', description: 'Generate seamless/tileable texture.' },
        gradientType: { type: 'string', enum: ['Linear', 'Radial', 'Angular'], description: 'Type of gradient.' },
        startColor: { type: 'object', description: 'Start color {r, g, b, a}.' },
        endColor: { type: 'object', description: 'End color {r, g, b, a}.' },
        angle: commonSchemas.angle,
        centerX: { type: 'number', description: 'Center X position (0-1) for radial gradient.' },
        centerY: { type: 'number', description: 'Center Y position (0-1) for radial gradient.' },
        radius: commonSchemas.radius,
        colorStops: { type: 'array', items: commonSchemas.objectProp, description: 'Array of {position, color} for multi-color gradients.' },
        patternType: { type: 'string', enum: ['Checker', 'Grid', 'Brick', 'Tile', 'Dots', 'Stripes'], description: 'Type of pattern.' },
        primaryColor: { type: 'object', description: 'Primary pattern color {r, g, b, a}.' },
        secondaryColor: { type: 'object', description: 'Secondary pattern color {r, g, b, a}.' },
        tilesX: { type: 'number', description: 'Number of pattern tiles horizontally.' },
        tilesY: { type: 'number', description: 'Number of pattern tiles vertically.' },
        lineWidth: { type: 'number', description: 'Line width for grid/stripes (0-1).' },
        brickRatio: { type: 'number', description: 'Width/height ratio for brick pattern.' },
        offset: { type: 'number', description: 'Brick offset ratio (0-1).' },
        sourceTexture: { type: 'string', description: 'Source height map texture path.' },
        strength: commonSchemas.strength,
        algorithm: { type: 'string', enum: ['Sobel', 'Prewitt', 'Scharr'], description: 'Normal calculation algorithm.' },
        flipY: { type: 'boolean', description: 'Flip green channel for DirectX/OpenGL compatibility.' },
        meshPath: commonSchemas.meshPath,
        samples: { type: 'number', description: 'Number of AO samples.' },
        rayDistance: { type: 'number', description: 'Maximum ray distance for AO.' },
        bias: { type: 'number', description: 'AO bias to prevent self-occlusion.' },
        uvChannel: { type: 'number', description: 'UV channel to use for baking.' },
        filterMethod: { type: 'string', enum: ['Nearest', 'Bilinear', 'Bicubic', 'Lanczos'], description: 'Resize filter method.' },
        preserveAspect: { type: 'boolean', description: 'Preserve aspect ratio when resizing.' },
        outputPath: commonSchemas.outputPath,
        inputBlackPoint: { type: 'number', description: 'Input black point (0-1).' },
        inputWhitePoint: { type: 'number', description: 'Input white point (0-1).' },
        gamma: { type: 'number', description: 'Gamma correction value.' },
        outputBlackPoint: { type: 'number', description: 'Output black point (0-1).' },
        outputWhitePoint: { type: 'number', description: 'Output white point (0-1).' },
        curvePoints: { type: 'array', items: commonSchemas.objectProp, description: 'Array of {x, y} curve control points.' },
        blurType: { type: 'string', enum: ['Gaussian', 'Box', 'Radial'], description: 'Type of blur.' },
        sharpenType: { type: 'string', enum: ['UnsharpMask', 'Laplacian'], description: 'Type of sharpening.' },
        channel: { type: 'string', enum: ['All', 'Red', 'Green', 'Blue', 'Alpha'], description: 'Target channel.' },
        invertAlpha: { type: 'boolean', description: 'Whether to invert alpha channel.' },
        amount: { type: 'number', description: 'Effect amount (0-1 for desaturate).' },
        method: { type: 'string', enum: ['Luminance', 'Average', 'Lightness'], description: 'Desaturation method.' },
        outputAsGrayscale: { type: 'boolean', description: 'Output extracted channel as grayscale.' },
        redChannel: { type: 'string', description: 'Source texture for red channel.' },
        greenChannel: { type: 'string', description: 'Source texture for green channel.' },
        blueChannel: { type: 'string', description: 'Source texture for blue channel.' },
        alphaChannel: { type: 'string', description: 'Source texture for alpha channel.' },
        redSourceChannel: { type: 'string', enum: ['Red', 'Green', 'Blue', 'Alpha'], description: 'Which channel to use from red source.' },
        greenSourceChannel: { type: 'string', enum: ['Red', 'Green', 'Blue', 'Alpha'], description: 'Which channel to use from green source.' },
        blueSourceChannel: { type: 'string', enum: ['Red', 'Green', 'Blue', 'Alpha'], description: 'Which channel to use from blue source.' },
        alphaSourceChannel: { type: 'string', enum: ['Red', 'Green', 'Blue', 'Alpha'], description: 'Which channel to use from alpha source.' },
        baseTexture: { type: 'string', description: 'Base texture path for combining.' },
        blendTexture: { type: 'string', description: 'Blend texture path for combining.' },
        blendMode: { type: 'string', enum: ['Multiply', 'Add', 'Subtract', 'Screen', 'Overlay', 'SoftLight', 'HardLight', 'Difference', 'Normal'], description: 'Blend mode for combining textures.' },
        opacity: { type: 'number', description: 'Blend opacity (0-1).' },
        maskTexture: { type: 'string', description: 'Optional mask texture for blending.' },
        compressionSettings: {
          type: 'string',
          enum: ['TC_Default', 'TC_Normalmap', 'TC_Masks', 'TC_Grayscale', 'TC_Displacementmap',
                 'TC_VectorDisplacementmap', 'TC_HDR', 'TC_EditorIcon', 'TC_Alpha',
                 'TC_DistanceFieldFont', 'TC_HDR_Compressed', 'TC_BC7'],
          description: 'Texture compression setting.'
        },
        textureGroup: {
          type: 'string',
          description: 'Texture group (TEXTUREGROUP_World, TEXTUREGROUP_Character, TEXTUREGROUP_UI, etc.).'
        },
        lodBias: { type: 'number', description: 'LOD bias (-2 to 4, lower = higher quality).' },
        virtualTextureStreaming: { type: 'boolean', description: 'Enable virtual texture streaming.' },
        tileSize: { type: 'number', description: 'Virtual texture tile size (32, 64, 128, 256, 512, 1024).' },
        tileBorderSize: { type: 'number', description: 'Virtual texture tile border size.' },
        neverStream: { type: 'boolean', description: 'Disable texture streaming.' },
        streamingPriority: { type: 'number', description: 'Streaming priority (-1 to 1, lower = higher priority).' },
        hdr: { type: 'boolean', description: 'Create HDR texture (16-bit float).' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.assetPath,
        textureInfo: {
          type: 'object',
          properties: {
            width: commonSchemas.numberProp,
            height: commonSchemas.numberProp,
            format: commonSchemas.stringProp,
            compression: commonSchemas.stringProp,
            textureGroup: commonSchemas.stringProp,
            mipCount: commonSchemas.numberProp,
            sRGB: commonSchemas.booleanProp,
            hasAlpha: commonSchemas.booleanProp,
            virtualTextureStreaming: commonSchemas.booleanProp,
            neverStream: commonSchemas.booleanProp
          }
        },
        error: commonSchemas.stringProp
      }
    }
  },
  // [MERGED] manage_animation_authoring actions now in animation_physics (Phase 53: Strategic Tool Merging)
  // [MERGED] manage_audio_authoring actions now in manage_audio (Phase 53: Strategic Tool Merging)
  // [MERGED] manage_niagara_authoring actions now in manage_effect (Phase 53: Strategic Tool Merging)
  {
    name: 'manage_gas',
    category: 'gameplay',
    description: 'Create Gameplay Abilities, Effects, Attribute Sets, and Gameplay Cues for ability systems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'add_ability_system_component',
            'configure_asc',
            'create_attribute_set',
            'add_attribute',
            'set_attribute_base_value',
            'set_attribute_clamping',
            'create_gameplay_ability',
            'set_ability_tags',
            'set_ability_costs',
            'set_ability_cooldown',
            'set_ability_targeting',
            'add_ability_task',
            'set_activation_policy',
            'set_instancing_policy',
            'create_gameplay_effect',
            'set_effect_duration',
            'add_effect_modifier',
            'set_modifier_magnitude',
            'add_effect_execution_calculation',
            'add_effect_cue',
            'set_effect_stacking',
            'set_effect_tags',
            'create_gameplay_cue_notify',
            'configure_cue_trigger',
            'set_cue_effects',
            'add_tag_to_asset',
            'get_gas_info'
          ],
          description: 'GAS action to perform.'
        },
        name: commonSchemas.assetNameForCreation,
        path: commonSchemas.directoryPathForCreation,
        assetPath: commonSchemas.assetPath,
        blueprintPath: commonSchemas.blueprintPath,
        save: commonSchemas.save,
        replicationMode: {
          type: 'string',
          enum: ['Full', 'Minimal', 'Mixed'],
          description: 'ASC replication mode.'
        },
        ownerActor: { type: 'string', description: 'Owner actor class for ASC.' },
        avatarActor: { type: 'string', description: 'Avatar actor class for ASC.' },
        attributeSetPath: { type: 'string', description: 'Path to Attribute Set asset.' },
        attributeName: commonSchemas.attributeName,
        attributeType: {
          type: 'string',
          enum: ['Health', 'MaxHealth', 'Mana', 'MaxMana', 'Stamina', 'MaxStamina', 'Damage', 'Armor', 'AttackPower', 'MoveSpeed', 'Custom'],
          description: 'Predefined attribute type or Custom.'
        },
        baseValue: { type: 'number', description: 'Base value for attribute.' },
        minValue: { type: 'number', description: 'Minimum value for clamping.' },
        maxValue: { type: 'number', description: 'Maximum value for clamping.' },
        clampMode: {
          type: 'string',
          enum: ['None', 'Min', 'Max', 'MinMax'],
          description: 'Attribute clamping mode.'
        },
        abilityPath: commonSchemas.abilityPath,
        parentClass: commonSchemas.parentClass,
        abilityTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Gameplay tags for this ability.'
        },
        cancelAbilitiesWithTag: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags of abilities to cancel when this activates.'
        },
        blockAbilitiesWithTag: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags of abilities blocked while this is active.'
        },
        activationRequiredTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags required to activate this ability.'
        },
        activationBlockedTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags that block activation of this ability.'
        },
        costEffectPath: { type: 'string', description: 'Path to cost Gameplay Effect.' },
        costAttribute: { type: 'string', description: 'Attribute used for cost (e.g., Mana).' },
        costMagnitude: { type: 'number', description: 'Cost magnitude.' },
        cooldownEffectPath: { type: 'string', description: 'Path to cooldown Gameplay Effect.' },
        cooldownDuration: { type: 'number', description: 'Cooldown duration in seconds.' },
        cooldownTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags applied during cooldown.'
        },
        targetingMode: {
          type: 'string',
          enum: ['None', 'SingleTarget', 'AOE', 'Directional', 'Ground', 'ActorPlacement'],
          description: 'Targeting mode for ability.'
        },
        targetRange: { type: 'number', description: 'Maximum targeting range.' },
        aoeRadius: { type: 'number', description: 'Area of effect radius.' },
        taskType: {
          type: 'string',
          enum: ['WaitDelay', 'WaitInputPress', 'WaitInputRelease', 'WaitGameplayEvent', 'WaitTargetData', 'WaitConfirmCancel', 'PlayMontageAndWait', 'ApplyRootMotionConstantForce', 'WaitMovementModeChange'],
          description: 'Type of ability task to add.'
        },
        taskSettings: {
          type: 'object',
          description: 'Task-specific settings.'
        },
        activationPolicy: {
          type: 'string',
          enum: ['OnInputPressed', 'WhileInputActive', 'OnSpawn', 'OnGiven'],
          description: 'When the ability activates.'
        },
        instancingPolicy: {
          type: 'string',
          enum: ['NonInstanced', 'InstancedPerActor', 'InstancedPerExecution'],
          description: 'How the ability is instanced.'
        },
        effectPath: commonSchemas.effectPath,
        durationType: {
          type: 'string',
          enum: ['Instant', 'Infinite', 'HasDuration'],
          description: 'Effect duration type.'
        },
        duration: commonSchemas.duration,
        period: { type: 'number', description: 'Period for periodic effects.' },
        modifierOperation: {
          type: 'string',
          enum: ['Add', 'Multiply', 'Divide', 'Override'],
          description: 'Modifier operation on attribute.'
        },
        modifierMagnitude: { type: 'number', description: 'Magnitude of the modifier.' },
        magnitudeCalculationType: {
          type: 'string',
          enum: ['ScalableFloat', 'AttributeBased', 'SetByCaller', 'CustomCalculationClass'],
          description: 'How magnitude is calculated.'
        },
        setByCallerTag: { type: 'string', description: 'Tag for SetByCaller magnitude.' },
        coefficient: { type: 'number', description: 'Coefficient for attribute-based calculation.' },
        preMultiplyAdditiveValue: { type: 'number', description: 'Value added before multiplication.' },
        postMultiplyAdditiveValue: { type: 'number', description: 'Value added after multiplication.' },
        sourceAttribute: { type: 'string', description: 'Source attribute for attribute-based calculation.' },
        targetAttribute: { type: 'string', description: 'Target attribute for modifier.' },
        calculationClass: { type: 'string', description: 'UGameplayEffectExecutionCalculation class path.' },
        cueTag: { type: 'string', description: 'Gameplay Cue tag (e.g., GameplayCue.Damage.Fire).' },
        cuePath: { type: 'string', description: 'Path to Gameplay Cue asset.' },
        stackingType: {
          type: 'string',
          enum: ['None', 'AggregateBySource', 'AggregateByTarget'],
          description: 'Stacking type for effect.'
        },
        stackLimitCount: { type: 'number', description: 'Maximum stack count.' },
        stackDurationRefreshPolicy: {
          type: 'string',
          enum: ['RefreshOnSuccessfulApplication', 'NeverRefresh'],
          description: 'When to refresh stack duration.'
        },
        stackPeriodResetPolicy: {
          type: 'string',
          enum: ['ResetOnSuccessfulApplication', 'NeverReset'],
          description: 'When to reset stack period.'
        },
        stackExpirationPolicy: {
          type: 'string',
          enum: ['ClearEntireStack', 'RemoveSingleStackAndRefreshDuration', 'RefreshDuration'],
          description: 'What happens when stack expires.'
        },
        grantedTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags granted while effect is active.'
        },
        applicationRequiredTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags required to apply this effect.'
        },
        removalTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags that cause effect removal.'
        },
        immunityTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags that block this effect.'
        },
        cueType: {
          type: 'string',
          enum: ['Static', 'Actor'],
          description: 'Type of gameplay cue notify.'
        },
        triggerType: {
          type: 'string',
          enum: ['OnActive', 'WhileActive', 'Executed', 'OnRemove'],
          description: 'When the cue triggers.'
        },
        particleSystemPath: commonSchemas.particleSystemPath,
        soundPath: commonSchemas.soundPath,
        cameraShakePath: commonSchemas.cameraShakePath,
        decalPath: commonSchemas.decalPath,
        tagName: commonSchemas.tagName,
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.assetPath,
        componentName: commonSchemas.componentName,
        attributeName: commonSchemas.stringProp,
        modifierIndex: commonSchemas.numberProp,
        gasInfo: {
          type: 'object',
          properties: {
            assetType: { type: 'string', enum: ['AttributeSet', 'GameplayAbility', 'GameplayEffect', 'GameplayCue'] },
            attributes: commonSchemas.arrayOfObjects,
            abilityTags: commonSchemas.arrayOfStrings,
            effectDuration: commonSchemas.stringProp,
            modifierCount: commonSchemas.numberProp,
            cueType: commonSchemas.stringProp
          }
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_character',
    category: 'gameplay',
    description: 'Create Character Blueprints with movement, locomotion, and animation state machines.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_character_blueprint',
            'configure_capsule_component',
            'configure_mesh_component',
            'configure_camera_component',
            'configure_movement_speeds',
            'configure_jump',
            'configure_rotation',
            'add_custom_movement_mode',
            'configure_nav_movement',
            'setup_mantling',
            'setup_vaulting',
            'setup_climbing',
            'setup_sliding',
            'setup_wall_running',
            'setup_grappling',
            'setup_footstep_system',
            'map_surface_to_sound',
            'configure_footstep_fx',
            'get_character_info'
          ],
          description: 'Character action to perform.'
        },
        name: commonSchemas.assetNameForCreation,
        path: commonSchemas.directoryPathForCreation,
        blueprintPath: commonSchemas.blueprintPath,
        save: commonSchemas.save,
        parentClass: {
          type: 'string',
          enum: ['Character', 'ACharacter', 'PlayerCharacter', 'AICharacter'],
          description: 'Parent class for character blueprint.'
        },
        skeletalMeshPath: commonSchemas.skeletalMeshPath,
        animBlueprintPath: commonSchemas.animBlueprintPath,
        capsuleRadius: commonSchemas.numberProp,
        capsuleHalfHeight: commonSchemas.numberProp,
        meshOffset: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp,
            y: commonSchemas.numberProp,
            z: commonSchemas.numberProp
          },
          description: 'Mesh location offset.'
        },
        meshRotation: {
          type: 'object',
          properties: {
            pitch: commonSchemas.numberProp,
            yaw: commonSchemas.numberProp,
            roll: commonSchemas.numberProp
          },
          description: 'Mesh rotation offset.'
        },
        cameraSocketName: commonSchemas.cameraSocketName,
        cameraOffset: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp,
            y: commonSchemas.numberProp,
            z: commonSchemas.numberProp
          },
          description: 'Camera location offset.'
        },
        cameraUsePawnControlRotation: { type: 'boolean', description: 'Camera follows controller rotation.' },
        springArmLength: commonSchemas.numberProp,
        springArmLagEnabled: { type: 'boolean', description: 'Enable camera lag.' },
        springArmLagSpeed: { type: 'number', description: 'Camera lag speed.' },
        walkSpeed: commonSchemas.numberProp,
        runSpeed: commonSchemas.numberProp,
        sprintSpeed: commonSchemas.numberProp,
        crouchSpeed: commonSchemas.numberProp,
        swimSpeed: commonSchemas.numberProp,
        flySpeed: commonSchemas.numberProp,
        acceleration: commonSchemas.numberProp,
        deceleration: commonSchemas.numberProp,
        groundFriction: commonSchemas.numberProp,
        jumpHeight: commonSchemas.numberProp,
        airControl: commonSchemas.numberProp,
        doubleJumpEnabled: { type: 'boolean', description: 'Enable double jump.' },
        maxJumpCount: commonSchemas.numberProp,
        jumpHoldTime: { type: 'number', description: 'Max hold time for variable jump.' },
        gravityScale: commonSchemas.numberProp,
        fallingLateralFriction: { type: 'number', description: 'Air friction.' },
        orientToMovement: { type: 'boolean', description: 'Orient rotation to movement direction.' },
        useControllerRotationYaw: { type: 'boolean', description: 'Use controller yaw rotation.' },
        useControllerRotationPitch: { type: 'boolean', description: 'Use controller pitch rotation.' },
        useControllerRotationRoll: { type: 'boolean', description: 'Use controller roll rotation.' },
        rotationRate: commonSchemas.numberProp,
        modeName: { type: 'string', description: 'Name for custom movement mode.' },
        modeId: { type: 'number', description: 'Custom movement mode ID.' },
        navAgentRadius: commonSchemas.numberProp,
        navAgentHeight: commonSchemas.numberProp,
        avoidanceEnabled: { type: 'boolean', description: 'Enable AI avoidance.' },
        pathFollowingEnabled: { type: 'boolean', description: 'Enable path following.' },
        mantleHeight: { type: 'number', description: 'Maximum mantle height.' },
        mantleReachDistance: { type: 'number', description: 'Forward reach for mantle check.' },
        mantleAnimationPath: { type: 'string', description: 'Path to mantle animation montage.' },
        vaultHeight: { type: 'number', description: 'Maximum vault obstacle height.' },
        vaultDepth: { type: 'number', description: 'Obstacle depth to check.' },
        vaultAnimationPath: { type: 'string', description: 'Path to vault animation montage.' },
        climbSpeed: commonSchemas.numberProp,
        climbableTag: { type: 'string', description: 'Tag for climbable surfaces.' },
        climbAnimationPath: { type: 'string', description: 'Path to climb animation.' },
        slideSpeed: commonSchemas.numberProp,
        slideDuration: commonSchemas.numberProp,
        slideCooldown: commonSchemas.numberProp,
        slideAnimationPath: { type: 'string', description: 'Path to slide animation.' },
        wallRunSpeed: { type: 'number', description: 'Wall running speed.' },
        wallRunDuration: { type: 'number', description: 'Maximum wall run duration.' },
        wallRunGravityScale: { type: 'number', description: 'Gravity during wall run.' },
        wallRunAnimationPath: { type: 'string', description: 'Path to wall run animation.' },
        grappleRange: { type: 'number', description: 'Maximum grapple distance.' },
        grappleSpeed: { type: 'number', description: 'Grapple pull speed.' },
        grappleTargetTag: { type: 'string', description: 'Tag for grapple targets.' },
        grappleCablePath: { type: 'string', description: 'Path to cable mesh/material.' },
        footstepEnabled: { type: 'boolean', description: 'Enable footstep system.' },
        footstepSocketLeft: { type: 'string', description: 'Left foot socket name.' },
        footstepSocketRight: { type: 'string', description: 'Right foot socket name.' },
        footstepTraceDistance: { type: 'number', description: 'Ground trace distance.' },
        surfaceType: {
          type: 'string',
          enum: ['Default', 'Concrete', 'Grass', 'Dirt', 'Metal', 'Wood', 'Water', 'Snow', 'Sand', 'Gravel', 'Custom'],
          description: 'Physical surface type.'
        },
        footstepSoundPath: { type: 'string', description: 'Path to footstep sound cue.' },
        footstepParticlePath: { type: 'string', description: 'Path to footstep particle.' },
        footstepDecalPath: { type: 'string', description: 'Path to footstep decal.' }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        blueprintPath: commonSchemas.blueprintPath,
        componentName: commonSchemas.componentName,
        modeName: commonSchemas.stringProp,
        characterInfo: {
          type: 'object',
          properties: {
            capsuleRadius: commonSchemas.numberProp,
            capsuleHalfHeight: commonSchemas.numberProp,
            walkSpeed: commonSchemas.numberProp,
            jumpZVelocity: commonSchemas.numberProp,
            airControl: commonSchemas.numberProp,
            orientToMovement: commonSchemas.booleanProp,
            hasSpringArm: commonSchemas.booleanProp,
            hasCamera: commonSchemas.booleanProp,
            customMovementModes: commonSchemas.arrayOfStrings
          }
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_combat',
    category: 'gameplay',
    description: 'Create weapons with hitscan/projectile firing, configure damage types, hitboxes, reload, and melee combat (combos, parry, block).',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_weapon_blueprint', 'configure_weapon_mesh', 'configure_weapon_sockets', 'set_weapon_stats',
            'configure_hitscan', 'configure_projectile', 'configure_spread_pattern', 'configure_recoil_pattern', 'configure_aim_down_sights',
            'create_projectile_blueprint', 'configure_projectile_movement', 'configure_projectile_collision', 'configure_projectile_homing',
            'create_damage_type', 'configure_damage_execution', 'setup_hitbox_component',
            'setup_reload_system', 'setup_ammo_system', 'setup_attachment_system', 'setup_weapon_switching',
            'configure_muzzle_flash', 'configure_tracer', 'configure_impact_effects', 'configure_shell_ejection',
            'create_melee_trace', 'configure_combo_system', 'create_hit_pause', 'configure_hit_reaction', 'setup_parry_block_system', 'configure_weapon_trails',
            'get_combat_info'
          ],
          description: 'Combat action to perform'
        },
        blueprintPath: commonSchemas.blueprintPath,
        name: commonSchemas.name,
        path: commonSchemas.directoryPathForCreation,
        weaponMeshPath: { type: 'string', description: 'Path to weapon static/skeletal mesh.' },
        muzzleSocketName: commonSchemas.muzzleSocketName,
        ejectionSocketName: commonSchemas.ejectionSocketName,
        attachmentSocketNames: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'List of attachment socket names.'
        },
        baseDamage: commonSchemas.numberProp,
        fireRate: commonSchemas.numberProp,
        range: commonSchemas.numberProp,
        spread: commonSchemas.numberProp,
        hitscanEnabled: { type: 'boolean', description: 'Enable hitscan firing.' },
        traceChannel: {
          type: 'string',
          enum: ['Visibility', 'Camera', 'Weapon', 'Custom'],
          description: 'Trace channel for hitscan.'
        },
        projectileClass: commonSchemas.projectileClass,
        spreadPattern: {
          type: 'string',
          enum: ['Random', 'Fixed', 'FixedWithRandom', 'Shotgun'],
          description: 'Spread pattern type.'
        },
        spreadIncrease: { type: 'number', description: 'Spread increase per shot.' },
        spreadRecovery: { type: 'number', description: 'Spread recovery rate.' },
        recoilPitch: { type: 'number', description: 'Vertical recoil (degrees).' },
        recoilYaw: { type: 'number', description: 'Horizontal recoil (degrees).' },
        recoilRecovery: { type: 'number', description: 'Recoil recovery speed.' },
        adsEnabled: { type: 'boolean', description: 'Enable aim down sights.' },
        adsFov: { type: 'number', description: 'FOV when aiming.' },
        adsSpeed: { type: 'number', description: 'Time to aim down sights.' },
        adsSpreadMultiplier: { type: 'number', description: 'Spread multiplier when aiming.' },
        projectileSpeed: commonSchemas.numberProp,
        projectileGravityScale: commonSchemas.numberProp,
        projectileLifespan: commonSchemas.numberProp,
        projectileMeshPath: { type: 'string', description: 'Path to projectile mesh.' },
        collisionRadius: commonSchemas.numberProp,
        bounceEnabled: { type: 'boolean', description: 'Enable projectile bouncing.' },
        bounceVelocityRatio: { type: 'number', description: 'Velocity retained on bounce (0-1).' },
        homingEnabled: { type: 'boolean', description: 'Enable homing behavior.' },
        homingAcceleration: { type: 'number', description: 'Homing turn rate.' },
        homingTargetTag: { type: 'string', description: 'Tag for homing targets.' },
        damageTypeName: { type: 'string', description: 'Name for damage type.' },
        damageCategory: {
          type: 'string',
          enum: ['Physical', 'Fire', 'Ice', 'Electric', 'Poison', 'Explosion', 'Radial', 'Custom'],
          description: 'Damage category.'
        },
        damageImpulse: { type: 'number', description: 'Impulse applied on hit.' },
        criticalMultiplier: { type: 'number', description: 'Critical hit damage multiplier.' },
        headshotMultiplier: { type: 'number', description: 'Headshot damage multiplier.' },
        hitboxBoneName: { type: 'string', description: 'Bone name for hitbox.' },
        hitboxType: {
          type: 'string',
          enum: ['Capsule', 'Box', 'Sphere'],
          description: 'Hitbox collision shape.'
        },
        hitboxSize: {
          type: 'object',
          properties: {
            radius: commonSchemas.numberProp,
            halfHeight: commonSchemas.numberProp,
            extent: commonSchemas.extent
          },
          description: 'Hitbox dimensions.'
        },
        isDamageZoneHead: { type: 'boolean', description: 'Mark as headshot zone.' },
        damageMultiplier: { type: 'number', description: 'Damage multiplier for this hitbox.' },
        magazineSize: commonSchemas.numberProp,
        reloadTime: commonSchemas.numberProp,
        reloadAnimationPath: { type: 'string', description: 'Path to reload animation.' },
        ammoType: { type: 'string', description: 'Ammo type identifier.' },
        maxAmmo: commonSchemas.numberProp,
        startingAmmo: commonSchemas.numberProp,
        attachmentSlots: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              slotName: commonSchemas.stringProp,
              socketName: commonSchemas.stringProp,
              allowedTypes: commonSchemas.arrayOfStrings
            }
          },
          description: 'Attachment slot definitions.'
        },
        switchInTime: { type: 'number', description: 'Time to equip weapon.' },
        switchOutTime: { type: 'number', description: 'Time to unequip weapon.' },
        switchInAnimationPath: { type: 'string', description: 'Path to equip animation.' },
        switchOutAnimationPath: { type: 'string', description: 'Path to unequip animation.' },
        muzzleFlashParticlePath: { type: 'string', description: 'Path to muzzle flash particle.' },
        muzzleFlashScale: { type: 'number', description: 'Muzzle flash scale.' },
        muzzleSoundPath: { type: 'string', description: 'Path to firing sound.' },
        tracerParticlePath: { type: 'string', description: 'Path to tracer particle.' },
        tracerSpeed: { type: 'number', description: 'Tracer travel speed.' },
        impactParticlePath: { type: 'string', description: 'Path to impact particle.' },
        impactSoundPath: { type: 'string', description: 'Path to impact sound.' },
        impactDecalPath: { type: 'string', description: 'Path to impact decal.' },
        shellMeshPath: { type: 'string', description: 'Path to shell casing mesh.' },
        shellEjectionForce: { type: 'number', description: 'Shell ejection impulse.' },
        shellLifespan: { type: 'number', description: 'Shell casing lifetime.' },
        meleeTraceStartSocket: { type: 'string', description: 'Socket for trace start.' },
        meleeTraceEndSocket: { type: 'string', description: 'Socket for trace end.' },
        meleeTraceRadius: { type: 'number', description: 'Sphere trace radius.' },
        meleeTraceChannel: { type: 'string', description: 'Trace channel for melee.' },
        comboWindowTime: { type: 'number', description: 'Time window for combo input.' },
        maxComboCount: { type: 'number', description: 'Maximum combo length.' },
        comboAnimations: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Paths to combo attack animations.'
        },
        hitPauseDuration: { type: 'number', description: 'Hitstop duration in seconds.' },
        hitPauseTimeDilation: { type: 'number', description: 'Time dilation during hitstop.' },
        hitReactionMontage: { type: 'string', description: 'Path to hit reaction montage.' },
        hitReactionStunTime: { type: 'number', description: 'Stun duration on hit.' },
        parryWindowStart: { type: 'number', description: 'Parry window start time (normalized).' },
        parryWindowEnd: { type: 'number', description: 'Parry window end time (normalized).' },
        parryAnimationPath: { type: 'string', description: 'Path to parry animation.' },
        blockDamageReduction: { type: 'number', description: 'Damage reduction when blocking (0-1).' },
        blockStaminaCost: { type: 'number', description: 'Stamina cost per blocked hit.' },
        weaponTrailParticlePath: { type: 'string', description: 'Path to weapon trail particle.' },
        weaponTrailStartSocket: { type: 'string', description: 'Trail start socket.' },
        weaponTrailEndSocket: { type: 'string', description: 'Trail end socket.' }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        blueprintPath: commonSchemas.blueprintPath,
        damageTypePath: commonSchemas.stringProp,
        combatInfo: {
          type: 'object',
          properties: {
            weaponType: commonSchemas.stringProp,
            firingMode: commonSchemas.stringProp,
            baseDamage: commonSchemas.numberProp,
            fireRate: commonSchemas.numberProp,
            magazineSize: commonSchemas.numberProp,
            hasADS: commonSchemas.booleanProp,
            hasReload: commonSchemas.booleanProp,
            isMelee: commonSchemas.booleanProp,
            comboCount: commonSchemas.numberProp,
            attachmentSlots: commonSchemas.arrayOfStrings
          },
          description: 'Combat configuration info (for get_combat_info).'
        }
      }
    }
  },
  {
    name: 'manage_ai',
    category: 'gameplay',
    description: 'Create AI Controllers, configure Behavior Trees, Blackboards, EQS queries, and perception systems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_ai_controller', 'assign_behavior_tree', 'assign_blackboard',
            'create_blackboard_asset', 'add_blackboard_key', 'set_key_instance_synced',
            'create_behavior_tree', 'add_composite_node', 'add_task_node', 'add_decorator', 'add_service', 'configure_bt_node',
            'create_eqs_query', 'add_eqs_generator', 'add_eqs_context', 'add_eqs_test', 'configure_test_scoring',
            'add_ai_perception_component', 'configure_sight_config', 'configure_hearing_config', 'configure_damage_sense_config', 'set_perception_team',
            'create_state_tree', 'add_state_tree_state', 'add_state_tree_transition', 'configure_state_tree_task',
            'create_smart_object_definition', 'add_smart_object_slot', 'configure_slot_behavior', 'add_smart_object_component',
            'create_mass_entity_config', 'configure_mass_entity', 'add_mass_spawner',
            'get_ai_info'
          ],
          description: 'AI action to perform'
        },
        name: commonSchemas.name,
        path: commonSchemas.directoryPathForCreation,
        blueprintPath: commonSchemas.blueprintPath,
        controllerPath: commonSchemas.controllerPath,
        behaviorTreePath: commonSchemas.behaviorTreePath,
        blackboardPath: commonSchemas.blackboardPath,
        parentClass: {
          type: 'string',
          enum: ['AAIController', 'APlayerController'],
          description: 'Parent class for AI controller (default: AAIController).'
        },
        autoRunBehaviorTree: { type: 'boolean', description: 'Start behavior tree automatically on possess.' },
        keyName: commonSchemas.keyName,
        keyType: {
          type: 'string',
          enum: ['Bool', 'Int', 'Float', 'Vector', 'Rotator', 'Object', 'Class', 'Enum', 'Name', 'String'],
          description: 'Blackboard key data type.'
        },
        isInstanceSynced: { type: 'boolean', description: 'Sync key across instances.' },
        baseObjectClass: { type: 'string', description: 'Base class for Object/Class keys.' },
        enumClass: { type: 'string', description: 'Enum class for Enum keys.' },
        compositeType: {
          type: 'string',
          enum: ['Selector', 'Sequence', 'Parallel', 'SimpleParallel'],
          description: 'Composite node type.'
        },
        taskType: {
          type: 'string',
          enum: [
            'MoveTo', 'MoveDirectlyToward', 'RotateToFaceBBEntry', 'Wait', 'WaitBlackboardTime',
            'PlayAnimation', 'PlaySound', 'RunEQSQuery', 'RunBehaviorDynamic', 'SetBlackboardValue',
            'PushPawnAction', 'FinishWithResult', 'MakeNoise', 'GameplayTaskBase', 'Custom'
          ],
          description: 'Task node type.'
        },
        decoratorType: {
          type: 'string',
          enum: [
            'Blackboard', 'BlackboardBased', 'CompareBBEntries', 'Cooldown', 'ConeCheck',
            'DoesPathExist', 'IsAtLocation', 'IsBBEntryOfClass', 'KeepInCone', 'Loop',
            'SetTagCooldown', 'TagCooldown', 'TimeLimit', 'ForceSuccess', 'ConditionalLoop', 'Custom'
          ],
          description: 'Decorator node type.'
        },
        serviceType: {
          type: 'string',
          enum: ['DefaultFocus', 'RunEQS', 'Custom'],
          description: 'Service node type.'
        },
        parentNodeId: commonSchemas.nodeId,
        nodeId: commonSchemas.nodeId,
        nodeProperties: {
          type: 'object',
          additionalProperties: true,
          description: 'Properties to set on the node.'
        },
        customTaskClass: { type: 'string', description: 'Custom task class path for Custom task type.' },
        customDecoratorClass: { type: 'string', description: 'Custom decorator class path.' },
        customServiceClass: { type: 'string', description: 'Custom service class path.' },
        queryPath: commonSchemas.queryPath,
        generatorType: {
          type: 'string',
          enum: ['ActorsOfClass', 'CurrentLocation', 'Donut', 'OnCircle', 'PathingGrid', 'SimpleGrid', 'Composite', 'Custom'],
          description: 'EQS generator type.'
        },
        contextType: {
          type: 'string',
          enum: ['Querier', 'Item', 'EnvQueryContext_BlueprintBase', 'Custom'],
          description: 'EQS context type.'
        },
        testType: {
          type: 'string',
          enum: ['Distance', 'Dot', 'GameplayTags', 'Overlap', 'Pathfinding', 'PathfindingBatch', 'Project', 'Random', 'Trace', 'Custom'],
          description: 'EQS test type.'
        },
        generatorSettings: {
          type: 'object',
          properties: {
            searchRadius: commonSchemas.numberProp,
            searchCenter: commonSchemas.stringProp,
            actorClass: commonSchemas.stringProp,
            gridSize: commonSchemas.numberProp,
            spacesBetween: commonSchemas.numberProp,
            innerRadius: commonSchemas.numberProp,
            outerRadius: commonSchemas.numberProp
          },
          description: 'Generator-specific settings.'
        },
        testSettings: {
          type: 'object',
          properties: {
            scoringEquation: { type: 'string', enum: ['Linear', 'Square', 'InverseLinear', 'Constant'] },
            clampMin: commonSchemas.numberProp,
            clampMax: commonSchemas.numberProp,
            filterType: { type: 'string', enum: ['Minimum', 'Maximum', 'Range'] },
            floatMin: commonSchemas.numberProp,
            floatMax: commonSchemas.numberProp
          },
          description: 'Test scoring and filter settings.'
        },
        testIndex: { type: 'number', description: 'Index of test to configure.' },
        sightConfig: {
          type: 'object',
          properties: {
            sightRadius: commonSchemas.numberProp,
            loseSightRadius: commonSchemas.numberProp,
            peripheralVisionAngle: commonSchemas.numberProp,
            pointOfViewBackwardOffset: commonSchemas.numberProp,
            nearClippingRadius: commonSchemas.numberProp,
            autoSuccessRange: commonSchemas.numberProp,
            maxAge: commonSchemas.numberProp,
            detectionByAffiliation: {
              type: 'object',
              properties: {
                enemies: commonSchemas.booleanProp,
                neutrals: commonSchemas.booleanProp,
                friendlies: commonSchemas.booleanProp
              }
            }
          },
          description: 'AI sight sense configuration.'
        },
        hearingConfig: {
          type: 'object',
          properties: {
            hearingRange: commonSchemas.numberProp,
            loSHearingRange: commonSchemas.numberProp,
            detectFriendly: commonSchemas.booleanProp,
            maxAge: commonSchemas.numberProp
          },
          description: 'AI hearing sense configuration.'
        },
        damageConfig: {
          type: 'object',
          properties: {
            maxAge: commonSchemas.numberProp
          },
          description: 'AI damage sense configuration.'
        },
        teamId: { type: 'number', description: 'Team ID for perception affiliation (0=Neutral, 1=Player, 2=Enemy, etc.).' },
        dominantSense: {
          type: 'string',
          enum: ['Sight', 'Hearing', 'Damage', 'Touch', 'None'],
          description: 'Dominant sense for perception prioritization.'
        },
        stateTreePath: commonSchemas.stateTreePath,
        stateName: commonSchemas.stateName,
        fromState: commonSchemas.fromState,
        toState: commonSchemas.toState,
        transitionCondition: { type: 'string', description: 'Condition expression for transition.' },
        stateTaskClass: { type: 'string', description: 'Task class for state.' },
        stateEvaluatorClass: { type: 'string', description: 'Evaluator class for state.' },
        definitionPath: commonSchemas.definitionPath,
        slotIndex: { type: 'number', description: 'Index of slot to configure.' },
        slotOffset: {
          type: 'object',
          properties: commonSchemas.vector3.properties,
          description: 'Local offset for slot.'
        },
        slotRotation: {
          type: 'object',
          properties: commonSchemas.rotation.properties,
          description: 'Local rotation for slot.'
        },
        slotBehaviorDefinition: { type: 'string', description: 'Gameplay behavior definition for slot.' },
        slotActivityTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Activity tags for the slot.'
        },
        slotUserTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Required user tags for slot.'
        },
        slotEnabled: { type: 'boolean', description: 'Whether slot is enabled.' },
        configPath: commonSchemas.configPath,
        massTraits: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'List of Mass traits to add.'
        },
        massProcessors: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'List of Mass processors to configure.'
        },
        spawnerSettings: {
          type: 'object',
          properties: {
            entityCount: commonSchemas.numberProp,
            spawnRadius: commonSchemas.numberProp,
            entityConfig: commonSchemas.stringProp,
            spawnOnBeginPlay: commonSchemas.booleanProp
          },
          description: 'Mass spawner configuration.'
        }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.assetPath,
        controllerPath: commonSchemas.stringProp,
        behaviorTreePath: commonSchemas.stringProp,
        blackboardPath: commonSchemas.stringProp,
        queryPath: commonSchemas.stringProp,
        stateTreePath: commonSchemas.stringProp,
        definitionPath: commonSchemas.stringProp,
        configPath: commonSchemas.stringProp,
        nodeId: commonSchemas.nodeId,
        keyIndex: commonSchemas.integerProp,
        testIndex: commonSchemas.integerProp,
        slotIndex: commonSchemas.integerProp,
        aiInfo: {
          type: 'object',
          properties: {
            controllerClass: commonSchemas.stringProp,
            assignedBehaviorTree: commonSchemas.stringProp,
            assignedBlackboard: commonSchemas.stringProp,
            blackboardKeys: commonSchemas.arrayOfObjects,
            btNodeCount: commonSchemas.integerProp,
            perceptionSenses: commonSchemas.arrayOfStrings,
            teamId: commonSchemas.integerProp,
            stateTreeStates: commonSchemas.arrayOfStrings,
            smartObjectSlots: commonSchemas.numberProp,
            massTraits: commonSchemas.arrayOfStrings
          },
          description: 'AI configuration info (for get_ai_info).'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_inventory',
    category: 'gameplay',
    description: 'Create item data assets, inventory components, world pickups, loot tables, and crafting recipes.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_item_data_asset',
            'set_item_properties',
            'create_item_category',
            'assign_item_category',
            'create_inventory_component',
            'configure_inventory_slots',
            'add_inventory_functions',
            'configure_inventory_events',
            'set_inventory_replication',
            'create_pickup_actor',
            'configure_pickup_interaction',
            'configure_pickup_respawn',
            'configure_pickup_effects',
            'create_equipment_component',
            'define_equipment_slots',
            'configure_equipment_effects',
            'add_equipment_functions',
            'configure_equipment_visuals',
            'create_loot_table',
            'add_loot_entry',
            'configure_loot_drop',
            'set_loot_quality_tiers',
            'create_crafting_recipe',
            'configure_recipe_requirements',
            'create_crafting_station',
            'add_crafting_component',
            'get_inventory_info'
          ],
          description: 'Inventory action to perform.'
        },
        name: commonSchemas.assetNameForCreation,
        path: commonSchemas.directoryPathForCreation,
        folder: commonSchemas.directoryPath,
        save: commonSchemas.save,
        blueprintPath: commonSchemas.blueprintPath,
        itemPath: commonSchemas.itemDataPath,
        parentClass: commonSchemas.parentClass,
        displayName: commonSchemas.stringProp,
        description: commonSchemas.stringProp,
        icon: commonSchemas.iconPath,
        mesh: commonSchemas.meshAssetPath,
        stackSize: commonSchemas.numberProp,
        weight: commonSchemas.numberProp,
        rarity: {
          type: 'string',
          enum: ['Common', 'Uncommon', 'Rare', 'Epic', 'Legendary', 'Custom'],
          description: 'Item rarity tier.'
        },
        value: commonSchemas.numberProp,
        tags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Gameplay tags for item categorization.'
        },
        customProperties: {
          type: 'object',
          additionalProperties: true,
          description: 'Custom key-value properties for item.'
        },
        categoryPath: { type: 'string', description: 'Path to item category asset.' },
        parentCategory: { type: 'string', description: 'Parent category path.' },
        categoryIcon: { type: 'string', description: 'Icon texture for category.' },
        componentName: commonSchemas.componentName,
        slotCount: commonSchemas.numberProp,
        slotSize: {
          type: 'object',
          properties: { width: commonSchemas.numberProp, height: commonSchemas.numberProp },
          description: 'Size of each slot (for grid inventory).'
        },
        maxWeight: commonSchemas.numberProp,
        allowStacking: { type: 'boolean', description: 'Allow items to stack.' },
        slotCategories: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Allowed item categories per slot.'
        },
        slotRestrictions: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              slotIndex: commonSchemas.numberProp,
              allowedCategories: commonSchemas.arrayOfStrings,
              restrictedCategories: commonSchemas.arrayOfStrings
            }
          },
          description: 'Per-slot category restrictions.'
        },
        replicated: commonSchemas.replicated,
        replicationCondition: {
          type: 'string',
          enum: ['None', 'OwnerOnly', 'SkipOwner', 'SimulatedOnly', 'AutonomousOnly', 'Custom'],
          description: 'Replication condition for inventory.'
        },
        pickupPath: { type: 'string', description: 'Path to pickup actor Blueprint.' },
        meshPath: commonSchemas.meshPath,
        itemDataPath: commonSchemas.itemDataPath,
        interactionRadius: { type: 'number', description: 'Radius for pickup interaction.' },
        interactionType: {
          type: 'string',
          enum: ['Overlap', 'Interact', 'Key', 'Hold'],
          description: 'How player picks up item.'
        },
        interactionKey: { type: 'string', description: 'Input action for pickup (if type is Key/Hold).' },
        prompt: commonSchemas.prompt,
        highlightMaterial: { type: 'string', description: 'Material for highlight effect.' },
        respawnable: commonSchemas.booleanProp,
        respawnTime: commonSchemas.respawnTime,
        respawnEffect: { type: 'string', description: 'Niagara effect for respawn.' },
        pickupSound: { type: 'string', description: 'Sound cue for pickup.' },
        pickupParticle: { type: 'string', description: 'Particle effect on pickup.' },
        bobbing: { type: 'boolean', description: 'Enable bobbing animation.' },
        rotation: { type: 'boolean', description: 'Enable rotation animation.' },
        glowEffect: { type: 'boolean', description: 'Enable glow effect.' },
        slots: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              name: commonSchemas.stringProp,
              socketName: commonSchemas.stringProp,
              allowedCategories: commonSchemas.arrayOfStrings,
              restrictedCategories: commonSchemas.arrayOfStrings
            }
          },
          description: 'Equipment slot definitions.'
        },
        statModifiers: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              attribute: commonSchemas.stringProp,
              operation: { type: 'string', enum: ['Add', 'Multiply', 'Override'] },
              value: commonSchemas.numberProp
            }
          },
          description: 'Stat modifiers when equipped.'
        },
        abilityGrants: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Gameplay abilities granted when equipped.'
        },
        passiveEffects: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Passive gameplay effects when equipped.'
        },
        attachToSocket: { type: 'boolean', description: 'Attach mesh to socket when equipped.' },
        meshComponent: { type: 'string', description: 'Component name for equipment mesh.' },
        animationOverrides: {
          type: 'object',
          additionalProperties: commonSchemas.stringProp,
          description: 'Animation overrides (slot -> anim asset).'
        },
        lootTablePath: commonSchemas.lootTablePath,
        lootWeight: { type: 'number', description: 'Weight for drop chance calculation.' },
        minQuantity: { type: 'number', description: 'Minimum drop quantity.' },
        maxQuantity: { type: 'number', description: 'Maximum drop quantity.' },
        conditions: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Conditions for loot entry (gameplay tag expressions).'
        },
        actorPath: { type: 'string', description: 'Path to actor Blueprint for loot drop.' },
        dropCount: { type: 'number', description: 'Number of drops to roll.' },
        guaranteedDrops: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Item paths that always drop.'
        },
        dropRadius: { type: 'number', description: 'Radius for scattered drops.' },
        dropForce: { type: 'number', description: 'Physics force applied to drops.' },
        tiers: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              name: commonSchemas.stringProp,
              color: {
                type: 'object',
                properties: commonSchemas.colorObject.properties
              },
              dropWeight: commonSchemas.numberProp,
              statMultiplier: commonSchemas.numberProp
            }
          },
          description: 'Quality tier definitions.'
        },
        recipePath: commonSchemas.recipePath,
        outputItemPath: { type: 'string', description: 'Path to item produced by recipe.' },
        outputQuantity: { type: 'number', description: 'Quantity produced.' },
        ingredients: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              itemPath: commonSchemas.stringProp,
              quantity: commonSchemas.numberProp
            }
          },
          description: 'Required ingredients with quantities.'
        },
        craftTime: { type: 'number', description: 'Time in seconds to craft.' },
        requiredLevel: { type: 'number', description: 'Required player level.' },
        requiredSkills: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Required skill tags.'
        },
        requiredStation: { type: 'string', description: 'Required crafting station type.' },
        unlockConditions: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Conditions to unlock recipe.'
        },
        recipes: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Recipe paths for crafting station.'
        },
        stationType: { type: 'string', description: 'Type of crafting station.' }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.assetPath,
        itemPath: commonSchemas.stringProp,
        categoryPath: commonSchemas.stringProp,
        pickupPath: commonSchemas.stringProp,
        lootTablePath: commonSchemas.stringProp,
        recipePath: commonSchemas.stringProp,
        stationPath: commonSchemas.stringProp,
        componentAdded: commonSchemas.booleanProp,
        slotCount: commonSchemas.integerProp,
        entryIndex: commonSchemas.integerProp,
        inventoryInfo: {
          type: 'object',
          properties: {
            assetType: { type: 'string', enum: ['Item', 'Inventory', 'Pickup', 'LootTable', 'Recipe', 'Station'] },
            itemProperties: {
              type: 'object',
              properties: {
                displayName: commonSchemas.stringProp,
                stackSize: commonSchemas.integerProp,
                weight: commonSchemas.numberProp,
                rarity: commonSchemas.stringProp,
                value: commonSchemas.numberProp
              }
            },
            inventorySlots: commonSchemas.numberProp,
            maxWeight: commonSchemas.numberProp,
            equipmentSlots: commonSchemas.arrayOfStrings,
            lootEntries: commonSchemas.numberProp,
            recipeIngredients: commonSchemas.arrayOfObjects,
            craftTime: commonSchemas.numberProp
          },
          description: 'Inventory system info (for get_inventory_info).'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_interaction',
    category: 'gameplay',
    description: 'Create interactive objects: doors, switches, chests, levers. Set up destructible meshes and trigger volumes.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_interaction_component',
            'configure_interaction_trace',
            'configure_interaction_widget',
            'add_interaction_events',
            'create_interactable_interface',
            'create_door_actor',
            'configure_door_properties',
            'create_switch_actor',
            'configure_switch_properties',
            'create_chest_actor',
            'configure_chest_properties',
            'create_lever_actor',
            'setup_destructible_mesh',
            'configure_destruction_levels',
            'configure_destruction_effects',
            'configure_destruction_damage',
            'add_destruction_component',
            'create_trigger_actor',
            'configure_trigger_events',
            'configure_trigger_filter',
            'configure_trigger_response',
            'get_interaction_info'
          ],
          description: 'The interaction action to perform.'
        },
        name: commonSchemas.name,
        folder: commonSchemas.directoryPath,
        blueprintPath: commonSchemas.blueprintPath,
        actorName: commonSchemas.actorName,
        componentName: commonSchemas.componentName,
        traceType: { type: 'string', enum: ['line', 'sphere', 'box'], description: 'Type of interaction trace.' },
        traceChannel: commonSchemas.traceChannel,
        traceDistance: commonSchemas.traceDistance,
        traceRadius: commonSchemas.traceRadius,
        traceFrequency: commonSchemas.traceFrequency,
        widgetClass: commonSchemas.widgetClass,
        widgetOffset: {
          type: 'object',
          properties: commonSchemas.vector3.properties,
          description: 'Widget offset from actor.'
        },
        showOnHover: { type: 'boolean', description: 'Show widget when hovering.' },
        showPromptText: { type: 'boolean', description: 'Show interaction prompt text.' },
        promptTextFormat: { type: 'string', description: 'Format string for prompt (e.g., "Press {Key} to {Action}").' },
        doorPath: { type: 'string', description: 'Path to door actor blueprint.' },
        meshPath: commonSchemas.meshPath,
        openAngle: { type: 'number', description: 'Door open rotation angle in degrees.' },
        openTime: { type: 'number', description: 'Time to open/close door in seconds.' },
        openDirection: { type: 'string', enum: ['push', 'pull', 'auto'], description: 'Door open direction.' },
        pivotOffset: {
          type: 'object',
          properties: commonSchemas.vector3.properties,
          description: 'Offset for door pivot point.'
        },
        locked: commonSchemas.locked,
        keyItemPath: { type: 'string', description: 'Item required to unlock.' },
        openSound: { type: 'string', description: 'Sound to play on open.' },
        closeSound: { type: 'string', description: 'Sound to play on close.' },
        autoClose: { type: 'boolean', description: 'Automatically close after opening.' },
        autoCloseDelay: { type: 'number', description: 'Delay before auto-close in seconds.' },
        requiresKey: { type: 'boolean', description: 'Whether interaction requires a key item.' },
        switchPath: { type: 'string', description: 'Path to switch actor blueprint.' },
        switchType: { type: 'string', enum: ['button', 'lever', 'pressure_plate', 'toggle'], description: 'Type of switch.' },
        toggleable: { type: 'boolean', description: 'Whether switch can be toggled.' },
        oneShot: { type: 'boolean', description: 'Whether switch can only be used once.' },
        resetTime: { type: 'number', description: 'Time to reset switch in seconds.' },
        activateSound: { type: 'string', description: 'Sound on activation.' },
        deactivateSound: { type: 'string', description: 'Sound on deactivation.' },
        targetActors: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Actors affected by this switch.'
        },
        chestPath: { type: 'string', description: 'Path to chest actor blueprint.' },
        lidMeshPath: { type: 'string', description: 'Path to lid mesh.' },
        lootTablePath: commonSchemas.lootTablePath,
        respawnable: commonSchemas.booleanProp,
        respawnTime: commonSchemas.respawnTime,
        leverType: { type: 'string', enum: ['rotate', 'translate'], description: 'Lever movement type.' },
        moveDistance: { type: 'number', description: 'Distance for translation lever.' },
        moveTime: { type: 'number', description: 'Time for lever movement.' },
        fractureMode: { type: 'string', enum: ['voronoi', 'uniform', 'radial', 'custom'], description: 'Fracture pattern type.' },
        fracturePieces: { type: 'number', description: 'Number of fracture pieces.' },
        enablePhysics: { type: 'boolean', description: 'Enable physics on destruction.' },
        levels: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              damageThreshold: commonSchemas.numberProp,
              meshIndex: commonSchemas.numberProp,
              enablePhysics: commonSchemas.booleanProp,
              removeAfterTime: commonSchemas.numberProp
            }
          },
          description: 'Destruction level definitions.'
        },
        destroySound: { type: 'string', description: 'Sound on destruction.' },
        destroyParticle: { type: 'string', description: 'Particle effect on destruction.' },
        debrisPhysicsMaterial: { type: 'string', description: 'Physics material for debris.' },
        debrisLifetime: { type: 'number', description: 'Debris lifetime in seconds.' },
        maxHealth: { type: 'number', description: 'Maximum health before destruction.' },
        damageThresholds: {
          type: 'array',
          items: commonSchemas.numberProp,
          description: 'Damage thresholds for destruction levels.'
        },
        impactDamageMultiplier: { type: 'number', description: 'Multiplier for impact damage.' },
        radialDamageMultiplier: { type: 'number', description: 'Multiplier for radial damage.' },
        autoDestroy: { type: 'boolean', description: 'Automatically destroy at zero health.' },
        triggerPath: { type: 'string', description: 'Path to trigger actor blueprint.' },
        triggerShape: { type: 'string', enum: ['box', 'sphere', 'capsule'], description: 'Shape of trigger volume.' },
        size: {
          type: 'object',
          properties: commonSchemas.vector3.properties,
          description: 'Size of trigger volume.'
        },
        filterByTag: { type: 'string', description: 'Actor tag filter for trigger.' },
        filterByClass: { type: 'string', description: 'Actor class filter for trigger.' },
        filterByInterface: { type: 'string', description: 'Interface filter for trigger.' },
        ignoreClasses: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Classes to ignore in trigger.'
        },
        ignoreTags: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Tags to ignore in trigger.'
        },
        onEnterEvent: { type: 'string', description: 'Event dispatcher name for enter.' },
        onExitEvent: { type: 'string', description: 'Event dispatcher name for exit.' },
        onStayEvent: { type: 'string', description: 'Event dispatcher name for stay.' },
        stayInterval: { type: 'number', description: 'Interval for stay events in seconds.' },
        responseType: { type: 'string', enum: ['once', 'repeatable', 'toggle'], description: 'How trigger responds.' },
        cooldown: commonSchemas.cooldown,
        maxActivations: { type: 'number', description: 'Maximum number of activations (0 = unlimited).' }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        success: commonSchemas.booleanProp,
        message: commonSchemas.stringProp,
        assetPath: commonSchemas.assetPath,
        blueprintPath: commonSchemas.blueprintPath,
        interfacePath: { type: 'string', description: 'Path to created interface.' },
        componentAdded: { type: 'boolean', description: 'Whether component was added.' },
        interactionInfo: {
          type: 'object',
          properties: {
            assetType: { type: 'string', enum: ['Door', 'Switch', 'Chest', 'Lever', 'Trigger', 'Destructible', 'Component'] },
            isLocked: commonSchemas.booleanProp,
            isOpen: commonSchemas.booleanProp,
            health: commonSchemas.numberProp,
            maxHealth: commonSchemas.numberProp,
            interactionEnabled: commonSchemas.booleanProp,
            triggerShape: commonSchemas.stringProp,
            destructionLevel: commonSchemas.numberProp
          },
          description: 'Interaction system info (for get_interaction_info).'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_widget_authoring',
    category: 'utility',
    description: 'Create UMG widgets: buttons, text, images, sliders. Configure layouts, bindings, animations. Build HUDs and menus.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_widget_blueprint',
            'set_widget_parent_class',
            'add_canvas_panel',
            'add_horizontal_box',
            'add_vertical_box',
            'add_overlay',
            'add_grid_panel',
            'add_uniform_grid',
            'add_wrap_box',
            'add_scroll_box',
            'add_size_box',
            'add_scale_box',
            'add_border',
            'add_text_block',
            'add_rich_text_block',
            'add_image',
            'add_button',
            'add_check_box',
            'add_slider',
            'add_progress_bar',
            'add_text_input',
            'add_combo_box',
            'add_spin_box',
            'add_list_view',
            'add_tree_view',
            'set_anchor',
            'set_alignment',
            'set_position',
            'set_size',
            'set_padding',
            'set_z_order',
            'set_render_transform',
            'set_visibility',
            'set_style',
            'set_clipping',
            'create_property_binding',
            'bind_text',
            'bind_visibility',
            'bind_color',
            'bind_enabled',
            'bind_on_clicked',
            'bind_on_hovered',
            'bind_on_value_changed',
            'create_widget_animation',
            'add_animation_track',
            'add_animation_keyframe',
            'set_animation_loop',
            'create_main_menu',
            'create_pause_menu',
            'create_settings_menu',
            'create_loading_screen',
            'create_hud_widget',
            'add_health_bar',
            'add_ammo_counter',
            'add_minimap',
            'add_crosshair',
            'add_compass',
            'add_interaction_prompt',
            'add_objective_tracker',
            'add_damage_indicator',
            'create_inventory_ui',
            'create_dialog_widget',
            'create_radial_menu',
            'get_widget_info',
            'preview_widget'
          ],
          description: 'The widget authoring action to perform.'
        },
        name: commonSchemas.name,
        folder: commonSchemas.directoryPath,
        widgetPath: commonSchemas.widgetPath,
        slotName: commonSchemas.slotName,
        parentSlot: { type: 'string', description: 'Parent slot to add widget to.' },
        parentClass: commonSchemas.parentClass,
        anchorMin: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'Minimum anchor point (0-1).'
        },
        anchorMax: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'Maximum anchor point (0-1).'
        },
        alignment: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'Widget alignment (0-1).'
        },
        alignmentX: { type: 'number', description: 'Horizontal alignment (0-1).' },
        alignmentY: { type: 'number', description: 'Vertical alignment (0-1).' },
        positionX: { type: 'number', description: 'X position.' },
        positionY: { type: 'number', description: 'Y position.' },
        sizeX: { type: 'number', description: 'Width.' },
        sizeY: { type: 'number', description: 'Height.' },
        sizeToContent: { type: 'boolean', description: 'Size to content.' },
        left: { type: 'number', description: 'Left padding.' },
        top: { type: 'number', description: 'Top padding.' },
        right: { type: 'number', description: 'Right padding.' },
        bottom: { type: 'number', description: 'Bottom padding.' },
        zOrder: { type: 'number', description: 'Z-order for canvas slot.' },
        translation: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'Render translation.'
        },
        scale: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'Render scale.'
        },
        shear: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'Render shear.'
        },
        angle: commonSchemas.angle,
        pivot: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'Rotation/scale pivot.'
        },
        visibility: {
          type: 'string',
          enum: ['Visible', 'Collapsed', 'Hidden', 'HitTestInvisible', 'SelfHitTestInvisible'],
          description: 'Widget visibility state.'
        },
        clipping: {
          type: 'string',
          enum: ['Inherit', 'ClipToBounds', 'ClipToBoundsWithoutIntersecting', 'ClipToBoundsAlways', 'OnDemand'],
          description: 'Widget clipping mode.'
        },
        text: commonSchemas.text,
        font: { type: 'string', description: 'Font asset path.' },
        fontSize: { type: 'number', description: 'Font size.' },
        colorAndOpacity: {
          type: 'object',
          properties: commonSchemas.colorObject.properties,
          description: 'Color and opacity (0-1 values).'
        },
        justification: {
          type: 'string',
          enum: ['Left', 'Center', 'Right'],
          description: 'Text justification.'
        },
        autoWrap: { type: 'boolean', description: 'Enable text auto-wrap.' },
        texturePath: commonSchemas.texturePath,
        brushSize: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'Brush/image size.'
        },
        brushTiling: {
          type: 'string',
          enum: ['NoTile', 'Horizontal', 'Vertical', 'Both'],
          description: 'Image tiling mode.'
        },
        isEnabled: { type: 'boolean', description: 'Widget enabled state.' },
        isChecked: { type: 'boolean', description: 'Checkbox checked state.' },
        value: { type: 'number', description: 'Slider/spinbox value.' },
        minValue: commonSchemas.minValue,
        maxValue: commonSchemas.maxValue,
        stepSize: { type: 'number', description: 'Value step size.' },
        delta: { type: 'number', description: 'Spinbox increment.' },
        percent: { type: 'number', description: 'Progress bar percentage (0-1).' },
        fillColorAndOpacity: {
          type: 'object',
          properties: commonSchemas.colorObject.properties,
          description: 'Fill color for progress bar.'
        },
        barFillType: {
          type: 'string',
          enum: ['LeftToRight', 'RightToLeft', 'TopToBottom', 'BottomToTop', 'FillFromCenter'],
          description: 'Progress bar fill direction.'
        },
        isMarquee: { type: 'boolean', description: 'Progress bar marquee mode.' },
        inputType: { type: 'string', enum: ['single', 'multi'], description: 'Text input type.' },
        hintText: { type: 'string', description: 'Placeholder hint text.' },
        isPassword: { type: 'boolean', description: 'Password masking.' },
        options: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Combo box options.'
        },
        selectedOption: { type: 'string', description: 'Selected combo box option.' },
        entryWidgetClass: { type: 'string', description: 'List/tree view entry widget class.' },
        orientation: {
          type: 'string',
          enum: ['Horizontal', 'Vertical'],
          description: 'Widget orientation.'
        },
        selectionMode: {
          type: 'string',
          enum: ['None', 'Single', 'Multi'],
          description: 'Selection mode for list/tree.'
        },
        scrollBarVisibility: {
          type: 'string',
          enum: ['Visible', 'Collapsed', 'Auto'],
          description: 'Scroll bar visibility.'
        },
        alwaysShowScrollbar: { type: 'boolean', description: 'Always show scrollbar.' },
        columnCount: { type: 'number', description: 'Number of columns.' },
        rowCount: { type: 'number', description: 'Number of rows.' },
        slotPadding: { type: 'number', description: 'Padding between slots.' },
        minDesiredSlotWidth: { type: 'number', description: 'Minimum slot width.' },
        minDesiredSlotHeight: { type: 'number', description: 'Minimum slot height.' },
        innerSlotPadding: { type: 'number', description: 'Inner slot padding.' },
        wrapWidth: { type: 'number', description: 'Wrap width for wrap box.' },
        explicitWrapWidth: { type: 'boolean', description: 'Use explicit wrap width.' },
        widthOverride: { type: 'number', description: 'Width override for size box.' },
        heightOverride: { type: 'number', description: 'Height override for size box.' },
        minDesiredWidth: { type: 'number', description: 'Minimum desired width.' },
        minDesiredHeight: { type: 'number', description: 'Minimum desired height.' },
        stretch: {
          type: 'string',
          enum: ['None', 'Fill', 'ScaleToFit', 'ScaleToFitX', 'ScaleToFitY', 'ScaleToFill', 'UserSpecified'],
          description: 'Scale box stretch mode.'
        },
        stretchDirection: {
          type: 'string',
          enum: ['Both', 'DownOnly', 'UpOnly'],
          description: 'Scale box stretch direction.'
        },
        userSpecifiedScale: { type: 'number', description: 'User specified scale value.' },
        brushColor: {
          type: 'object',
          properties: commonSchemas.colorObject.properties,
          description: 'Border brush color.'
        },
        padding: { type: 'number', description: 'Uniform padding.' },
        horizontalAlignment: {
          type: 'string',
          enum: ['Fill', 'Left', 'Center', 'Right'],
          description: 'Horizontal alignment.'
        },
        verticalAlignment: {
          type: 'string',
          enum: ['Fill', 'Top', 'Center', 'Bottom'],
          description: 'Vertical alignment.'
        },
        color: {
          type: 'object',
          properties: commonSchemas.colorObject.properties,
          description: 'Widget color.'
        },
        opacity: { type: 'number', description: 'Widget opacity (0-1).' },
        brush: { type: 'string', description: 'Brush asset path.' },
        backgroundImage: { type: 'string', description: 'Background image path.' },
        style: { type: 'string', description: 'Style preset name.' },
        propertyName: commonSchemas.propertyName,
        bindingType: { type: 'string', enum: ['function', 'variable'], description: 'Binding type.' },
        bindingSource: { type: 'string', description: 'Variable or function name to bind to.' },
        functionName: commonSchemas.functionName,
        onHoveredFunction: { type: 'string', description: 'Function to call on hover.' },
        onUnhoveredFunction: { type: 'string', description: 'Function to call on unhover.' },
        animationName: commonSchemas.animationName,
        length: { type: 'number', description: 'Animation length in seconds.' },
        trackType: {
          type: 'string',
          enum: ['transform', 'color', 'opacity', 'renderOpacity', 'material'],
          description: 'Animation track type.'
        },
        time: { type: 'number', description: 'Keyframe time.' },
        interpolation: {
          type: 'string',
          enum: ['linear', 'cubic', 'constant'],
          description: 'Keyframe interpolation.'
        },
        loopCount: { type: 'number', description: 'Number of loops (-1 for infinite).' },
        playMode: {
          type: 'string',
          enum: ['forward', 'reverse', 'pingpong'],
          description: 'Animation play mode.'
        },
        includePlayButton: { type: 'boolean', description: 'Include play button in menu.' },
        includeSettingsButton: { type: 'boolean', description: 'Include settings button.' },
        includeQuitButton: { type: 'boolean', description: 'Include quit button.' },
        includeResumeButton: { type: 'boolean', description: 'Include resume button.' },
        includeQuitToMenuButton: { type: 'boolean', description: 'Include quit to menu button.' },
        settingsType: {
          type: 'string',
          enum: ['video', 'audio', 'controls', 'gameplay', 'all'],
          description: 'Settings menu type.'
        },
        includeApplyButton: { type: 'boolean', description: 'Include apply button.' },
        includeResetButton: { type: 'boolean', description: 'Include reset button.' },
        includeProgressBar: { type: 'boolean', description: 'Include progress bar.' },
        includeTipText: { type: 'boolean', description: 'Include tip text.' },
        includeBackgroundImage: { type: 'boolean', description: 'Include background image.' },
        titleText: { type: 'string', description: 'Menu title text.' },
        elements: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'HUD elements to include.'
        },
        barStyle: {
          type: 'string',
          enum: ['simple', 'segmented', 'radial'],
          description: 'Health bar style.'
        },
        showNumbers: { type: 'boolean', description: 'Show numeric values.' },
        barColor: {
          type: 'object',
          properties: commonSchemas.colorObject.properties,
          description: 'Bar color.'
        },
        ammoStyle: {
          type: 'string',
          enum: ['numeric', 'icon'],
          description: 'Ammo counter style.'
        },
        showReserve: { type: 'boolean', description: 'Show reserve ammo.' },
        ammoIcon: { type: 'string', description: 'Ammo icon texture.' },
        minimapSize: { type: 'number', description: 'Minimap size.' },
        minimapShape: {
          type: 'string',
          enum: ['circle', 'square'],
          description: 'Minimap shape.'
        },
        rotateWithPlayer: { type: 'boolean', description: 'Rotate minimap with player.' },
        showObjectives: { type: 'boolean', description: 'Show objectives on minimap.' },
        crosshairStyle: {
          type: 'string',
          enum: ['dot', 'cross', 'circle', 'custom'],
          description: 'Crosshair style.'
        },
        crosshairSize: { type: 'number', description: 'Crosshair size.' },
        spreadMultiplier: { type: 'number', description: 'Crosshair spread multiplier.' },
        showDegrees: { type: 'boolean', description: 'Show compass degrees.' },
        showCardinals: { type: 'boolean', description: 'Show cardinal directions.' },
        promptFormat: { type: 'string', description: 'Interaction prompt format.' },
        showKeyIcon: { type: 'boolean', description: 'Show key icon in prompt.' },
        keyIconStyle: { type: 'string', description: 'Key icon style.' },
        maxVisibleObjectives: { type: 'number', description: 'Maximum visible objectives.' },
        showProgress: { type: 'boolean', description: 'Show objective progress.' },
        animateUpdates: { type: 'boolean', description: 'Animate objective updates.' },
        indicatorStyle: {
          type: 'string',
          enum: ['directional', 'vignette', 'both'],
          description: 'Damage indicator style.'
        },
        fadeTime: commonSchemas.fadeTime,
        gridSize: {
          type: 'object',
          properties: { columns: commonSchemas.numberProp, rows: commonSchemas.numberProp },
          description: 'Inventory grid size.'
        },
        slotSize: { type: 'number', description: 'Inventory slot size.' },
        showEquipment: { type: 'boolean', description: 'Show equipment panel.' },
        showDetails: { type: 'boolean', description: 'Show item details panel.' },
        showPortrait: { type: 'boolean', description: 'Show speaker portrait.' },
        showSpeakerName: { type: 'boolean', description: 'Show speaker name.' },
        choiceLayout: {
          type: 'string',
          enum: ['vertical', 'horizontal', 'radial'],
          description: 'Dialog choice layout.'
        },
        segmentCount: { type: 'number', description: 'Number of radial segments.' },
        innerRadius: { type: 'number', description: 'Inner radius of radial menu.' },
        outerRadius: { type: 'number', description: 'Outer radius of radial menu.' },
        showIcons: { type: 'boolean', description: 'Show icons in radial menu.' },
        showLabels: { type: 'boolean', description: 'Show labels in radial menu.' },
        previewSize: {
          type: 'string',
          enum: ['1080p', '720p', 'mobile', 'custom'],
          description: 'Preview resolution preset.'
        },
        customWidth: { type: 'number', description: 'Custom preview width.' },
        customHeight: { type: 'number', description: 'Custom preview height.' }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        success: commonSchemas.booleanProp,
        message: commonSchemas.stringProp,
        widgetPath: commonSchemas.widgetPath,
        slotName: commonSchemas.slotName,
        animationName: commonSchemas.animationName,
        trackIndex: { type: 'number', description: 'Index of created track.' },
        keyframeIndex: { type: 'number', description: 'Index of created keyframe.' },
        bindingCreated: { type: 'boolean', description: 'Whether binding was created.' },
        widgetInfo: {
          type: 'object',
          properties: {
            widgetClass: commonSchemas.stringProp,
            parentClass: commonSchemas.stringProp,
            slots: commonSchemas.arrayOfStrings,
            animations: commonSchemas.arrayOfStrings,
            variables: commonSchemas.arrayOfStrings,
            functions: commonSchemas.arrayOfStrings,
            eventDispatchers: commonSchemas.arrayOfStrings
          },
          description: 'Widget info (for get_widget_info).'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_networking',
    category: 'utility',
    description: 'Configure multiplayer: property replication, RPCs (Server/Client/Multicast), authority, relevancy, and network prediction.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'set_property_replicated',
            'set_replication_condition',
            'configure_net_update_frequency',
            'configure_net_priority',
            'set_net_dormancy',
            'configure_replication_graph',
            'create_rpc_function',
            'configure_rpc_validation',
            'set_rpc_reliability',
            'set_owner',
            'set_autonomous_proxy',
            'check_has_authority',
            'check_is_locally_controlled',
            'configure_net_cull_distance',
            'set_always_relevant',
            'set_only_relevant_to_owner',
            'configure_net_serialization',
            'set_replicated_using',
            'configure_push_model',
            'configure_client_prediction',
            'configure_server_correction',
            'add_network_prediction_data',
            'configure_movement_prediction',
            'configure_net_driver',
            'set_net_role',
            'configure_replicated_movement',
            'get_networking_info'
          ],
          description: 'Networking action to perform'
        },
        blueprintPath: commonSchemas.blueprintPath,
        actorName: commonSchemas.actorName,
        propertyName: commonSchemas.propertyName,
        replicated: { type: 'boolean', description: 'Whether property should be replicated.' },
        condition: {
          type: 'string',
          enum: [
            'COND_None',
            'COND_InitialOnly',
            'COND_OwnerOnly',
            'COND_SkipOwner',
            'COND_SimulatedOnly',
            'COND_AutonomousOnly',
            'COND_SimulatedOrPhysics',
            'COND_InitialOrOwner',
            'COND_Custom',
            'COND_ReplayOrOwner',
            'COND_ReplayOnly',
            'COND_SimulatedOnlyNoReplay',
            'COND_SimulatedOrPhysicsNoReplay',
            'COND_SkipReplay',
            'COND_Never'
          ],
          description: 'Replication condition.'
        },
        repNotifyFunc: commonSchemas.repNotifyFunc,
        netUpdateFrequency: { type: 'number', description: 'How often actor replicates (Hz, default 100).' },
        minNetUpdateFrequency: { type: 'number', description: 'Minimum update frequency when idle (Hz, default 2).' },
        netPriority: { type: 'number', description: 'Network priority for bandwidth (default 1.0).' },
        dormancy: {
          type: 'string',
          enum: [
            'DORM_Never',
            'DORM_Awake',
            'DORM_DormantAll',
            'DORM_DormantPartial',
            'DORM_Initial'
          ],
          description: 'Net dormancy mode.'
        },
        nodeClass: commonSchemas.nodeClass,
        spatialBias: { type: 'number', description: 'Spatial bias for replication graph.' },
        defaultSettingsClass: { type: 'string', description: 'Default replication settings class.' },
        functionName: commonSchemas.functionName,
        rpcType: {
          type: 'string',
          enum: ['Server', 'Client', 'NetMulticast'],
          description: 'Type of RPC.'
        },
        reliable: commonSchemas.reliable,
        parameters: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              name: commonSchemas.stringProp,
              type: commonSchemas.stringProp
            }
          },
          description: 'RPC function parameters.'
        },
        returnType: { type: 'string', description: 'RPC return type (usually void).' },
        validationFunctionName: { type: 'string', description: 'Name of validation function.' },
        withValidation: { type: 'boolean', description: 'Enable RPC validation.' },
        ownerActorName: { type: 'string', description: 'Name of owner actor (null to clear).' },
        isAutonomousProxy: { type: 'boolean', description: 'Configure as autonomous proxy.' },
        netCullDistanceSquared: { type: 'number', description: 'Network cull distance squared.' },
        useOwnerNetRelevancy: { type: 'boolean', description: 'Use owner relevancy.' },
        alwaysRelevant: { type: 'boolean', description: 'Always relevant to all clients.' },
        onlyRelevantToOwner: { type: 'boolean', description: 'Only relevant to owner.' },
        structName: { type: 'string', description: 'Name of struct for custom serialization.' },
        useNetSerialize: { type: 'boolean', description: 'Use custom NetSerialize.' },
        usePushModel: { type: 'boolean', description: 'Use push-model replication.' },
        propertyNames: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Properties for push model.'
        },
        enablePrediction: { type: 'boolean', description: 'Enable client-side prediction.' },
        predictionKey: { type: 'string', description: 'Prediction key identifier.' },
        correctionThreshold: { type: 'number', description: 'Server correction threshold.' },
        smoothingRate: { type: 'number', description: 'Smoothing rate for corrections.' },
        dataType: {
          type: 'string',
          enum: ['InputCmd', 'SyncState', 'AuxState'],
          description: 'Network prediction data type.'
        },
        properties: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              name: commonSchemas.stringProp,
              type: commonSchemas.stringProp
            }
          },
          description: 'Predicted properties.'
        },
        networkSmoothingMode: {
          type: 'string',
          enum: ['Disabled', 'Linear', 'Exponential'],
          description: 'Movement smoothing mode.'
        },
        networkMaxSmoothUpdateDistance: { type: 'number', description: 'Max smooth update distance.' },
        networkNoSmoothUpdateDistance: { type: 'number', description: 'No smooth update distance.' },
        maxClientRate: { type: 'number', description: 'Max client rate.' },
        maxInternetClientRate: { type: 'number', description: 'Max internet client rate.' },
        netServerMaxTickRate: { type: 'number', description: 'Server max tick rate.' },
        role: {
          type: 'string',
          enum: ['ROLE_None', 'ROLE_SimulatedProxy', 'ROLE_AutonomousProxy', 'ROLE_Authority'],
          description: 'Net role.'
        },
        replicateMovement: { type: 'boolean', description: 'Replicate movement.' },
        replicatedMovementMode: {
          type: 'string',
          enum: ['Default', 'SkipPhysics', 'FullMovement'],
          description: 'Replicated movement mode.'
        },
        locationQuantizationLevel: {
          type: 'string',
          enum: ['RoundWholeNumber', 'RoundOneDecimal', 'RoundTwoDecimals'],
          description: 'Location quantization level.'
        },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        success: commonSchemas.booleanProp,
        message: commonSchemas.stringProp,
        blueprintPath: commonSchemas.blueprintPath,
        functionName: { type: 'string', description: 'Created RPC function name.' },
        hasAuthority: { type: 'boolean', description: 'Authority check result.' },
        isLocallyControlled: { type: 'boolean', description: 'Local control check result.' },
        role: { type: 'string', description: 'Current net role.' },
        remoteRole: { type: 'string', description: 'Current remote role.' },
        networkingInfo: {
          type: 'object',
          properties: {
            bReplicates: commonSchemas.booleanProp,
            bAlwaysRelevant: commonSchemas.booleanProp,
            bOnlyRelevantToOwner: commonSchemas.booleanProp,
            netUpdateFrequency: commonSchemas.numberProp,
            minNetUpdateFrequency: commonSchemas.numberProp,
            netPriority: commonSchemas.numberProp,
            netDormancy: commonSchemas.stringProp,
            netCullDistanceSquared: commonSchemas.numberProp,
            replicatedProperties: {
              type: 'array',
              items: {
                type: 'object',
                properties: {
                  name: commonSchemas.stringProp,
                  condition: commonSchemas.stringProp,
                  repNotifyFunc: commonSchemas.stringProp
                }
              }
            },
            rpcFunctions: {
              type: 'array',
              items: {
                type: 'object',
                properties: {
                  name: commonSchemas.stringProp,
                  type: commonSchemas.stringProp,
                  reliable: commonSchemas.booleanProp
                }
              }
            }
          },
          description: 'Networking info (for get_networking_info).'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_game_framework',
    category: 'utility',
    description: 'Create GameMode, GameState, PlayerController, PlayerState Blueprints. Configure match flow, teams, scoring, and spawning.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_game_mode', 'create_game_state', 'create_player_controller',
            'create_player_state', 'create_game_instance', 'create_hud_class',
            'set_default_pawn_class', 'set_player_controller_class',
            'set_game_state_class', 'set_player_state_class', 'configure_game_rules',
            'setup_match_states', 'configure_round_system', 'configure_team_system',
            'configure_scoring_system', 'configure_spawn_system',
            'configure_player_start', 'set_respawn_rules', 'configure_spectating',
            'get_game_framework_info'
          ],
          description: 'Game framework action to perform.'
        },
        name: commonSchemas.name,
        path: commonSchemas.directoryPathForCreation,
        gameModeBlueprint: { type: 'string', description: 'Path to GameMode blueprint to configure.' },
        blueprintPath: commonSchemas.blueprintPath,
        levelPath: commonSchemas.levelPath,
        parentClass: commonSchemas.parentClass,
        pawnClass: { type: 'string', description: 'Pawn class to use.' },
        defaultPawnClass: { type: 'string', description: 'Default pawn class for GameMode.' },
        playerControllerClass: { type: 'string', description: 'PlayerController class path.' },
        gameStateClass: { type: 'string', description: 'GameState class path.' },
        playerStateClass: { type: 'string', description: 'PlayerState class path.' },
        spectatorClass: { type: 'string', description: 'Spectator pawn class.' },
        hudClass: { type: 'string', description: 'HUD class path.' },
        timeLimit: commonSchemas.numberProp,
        scoreLimit: commonSchemas.numberProp,
        bDelayedStart: { type: 'boolean', description: 'Whether to delay match start.' },
        startPlayersNeeded: commonSchemas.numberProp,
        states: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              name: { type: 'string', enum: ['waiting', 'warmup', 'in_progress', 'post_match', 'custom'] },
              duration: commonSchemas.duration,
              customName: { type: 'string', description: 'Custom state name if name is "custom".' }
            }
          },
          description: 'Match state definitions.'
        },
        numRounds: commonSchemas.numberProp,
        roundTime: commonSchemas.numberProp,
        intermissionTime: commonSchemas.numberProp,
        numTeams: commonSchemas.numberProp,
        teamSize: commonSchemas.numberProp,
        autoBalance: { type: 'boolean', description: 'Enable automatic team balancing.' },
        friendlyFire: { type: 'boolean', description: 'Enable friendly fire damage.' },
        teamIndex: { type: 'number', description: 'Team index for PlayerStart.' },
        scorePerKill: { type: 'number', description: 'Points awarded per kill.' },
        scorePerObjective: { type: 'number', description: 'Points awarded per objective.' },
        scorePerAssist: { type: 'number', description: 'Points awarded per assist.' },
        spawnSelectionMethod: {
          type: 'string',
          enum: ['Random', 'RoundRobin', 'FarthestFromEnemies'],
          description: 'How to select spawn points.'
        },
        respawnDelay: commonSchemas.numberProp,
        respawnLocation: {
          type: 'string',
          enum: ['PlayerStart', 'LastDeath', 'TeamBase'],
          description: 'Where players respawn.'
        },
        respawnConditions: {
          type: 'array',
          items: commonSchemas.stringProp,
          description: 'Conditions for respawn (e.g., "RoundEnd", "Manual").'
        },
        usePlayerStarts: { type: 'boolean', description: 'Use PlayerStart actors.' },
        location: {
          type: 'object',
          properties: commonSchemas.vector3.properties,
          description: 'Spawn location.'
        },
        rotation: {
          type: 'object',
          properties: commonSchemas.rotation.properties,
          description: 'Spawn rotation.'
        },
        bPlayerOnly: { type: 'boolean', description: 'Restrict to players only.' },
        allowSpectating: { type: 'boolean', description: 'Allow spectator mode.' },
        spectatorViewMode: {
          type: 'string',
          enum: ['FreeCam', 'ThirdPerson', 'FirstPerson', 'DeathCam'],
          description: 'Spectator view mode.'
        },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        success: commonSchemas.booleanProp,
        message: commonSchemas.stringProp,
        blueprintPath: commonSchemas.blueprintPath,
        gameFrameworkInfo: {
          type: 'object',
          properties: {
            gameModeClass: commonSchemas.stringProp,
            gameStateClass: commonSchemas.stringProp,
            playerControllerClass: commonSchemas.stringProp,
            playerStateClass: commonSchemas.stringProp,
            defaultPawnClass: commonSchemas.stringProp,
            hudClass: commonSchemas.stringProp,
            spectatorClass: commonSchemas.stringProp,
            matchState: commonSchemas.stringProp,
            numTeams: commonSchemas.numberProp,
            timeLimit: commonSchemas.numberProp,
            scoreLimit: commonSchemas.numberProp
          },
          description: 'Game framework information (for get_game_framework_info).'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_sessions',
    category: 'utility',
    description: 'Configure local multiplayer: split-screen layouts, LAN hosting/joining, voice chat channels, and push-to-talk.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'configure_local_session_settings', 'configure_session_interface',
            'configure_split_screen', 'set_split_screen_type', 'add_local_player', 'remove_local_player',
            'configure_lan_play', 'host_lan_server', 'join_lan_server',
            'enable_voice_chat', 'configure_voice_settings', 'set_voice_channel',
            'mute_player', 'set_voice_attenuation', 'configure_push_to_talk',
            'get_sessions_info'
          ],
          description: 'Sessions action to perform.'
        },
        sessionName: commonSchemas.sessionName,
        sessionId: commonSchemas.sessionId,
        maxPlayers: commonSchemas.numberProp,
        bIsLANMatch: { type: 'boolean', description: 'Whether this is a LAN match.' },
        bAllowJoinInProgress: { type: 'boolean', description: 'Allow joining games in progress.' },
        bAllowInvites: { type: 'boolean', description: 'Allow player invites.' },
        bUsesPresence: { type: 'boolean', description: 'Use presence for session discovery.' },
        bUseLobbiesIfAvailable: { type: 'boolean', description: 'Use lobby system if available.' },
        bShouldAdvertise: { type: 'boolean', description: 'Advertise session publicly.' },
        interfaceType: {
          type: 'string',
          enum: ['Default', 'LAN', 'Null'],
          description: 'Type of session interface to use.'
        },
        enabled: commonSchemas.enabled,
        splitScreenType: {
          type: 'string',
          enum: ['None', 'TwoPlayer_Horizontal', 'TwoPlayer_Vertical', 'ThreePlayer_FavorTop', 'ThreePlayer_FavorBottom', 'FourPlayer_Grid'],
          description: 'Split-screen layout type.'
        },
        playerIndex: commonSchemas.numberProp,
        controllerId: commonSchemas.numberProp,
        serverAddress: commonSchemas.serverAddress,
        serverPort: commonSchemas.numberProp,
        serverPassword: { type: 'string', description: 'Server password for protected games.' },
        serverName: { type: 'string', description: 'Display name for the server.' },
        mapName: { type: 'string', description: 'Map to load for hosting.' },
        travelOptions: { type: 'string', description: 'Travel URL options string.' },
        voiceEnabled: { type: 'boolean', description: 'Enable/disable voice chat.' },
        voiceSettings: {
          type: 'object',
          properties: {
            volume: { type: 'number', description: 'Voice volume (0.0 - 1.0).' },
            noiseGateThreshold: { type: 'number', description: 'Noise gate threshold.' },
            noiseSuppression: { type: 'boolean', description: 'Enable noise suppression.' },
            echoCancellation: { type: 'boolean', description: 'Enable echo cancellation.' },
            sampleRate: { type: 'number', description: 'Audio sample rate in Hz.' }
          },
          description: 'Voice processing settings.'
        },
        channelName: commonSchemas.channelName,
        channelType: {
          type: 'string',
          enum: ['Team', 'Global', 'Proximity', 'Party'],
          description: 'Voice channel type.'
        },
        playerName: { type: 'string', description: 'Player name for voice operations.' },
        targetPlayerId: { type: 'string', description: 'Target player ID.' },
        muted: commonSchemas.muted,
        attenuationRadius: { type: 'number', description: 'Radius for voice attenuation (Proximity chat).' },
        attenuationFalloff: { type: 'number', description: 'Falloff rate for voice attenuation.' },
        pushToTalkEnabled: { type: 'boolean', description: 'Enable push-to-talk mode.' },
        pushToTalkKey: { type: 'string', description: 'Key binding for push-to-talk.' }
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        success: commonSchemas.booleanProp,
        message: commonSchemas.stringProp,
        sessionId: commonSchemas.sessionId,
        sessionName: { type: 'string', description: 'Name of created session.' },
        playerIndex: { type: 'number', description: 'Index of added local player.' },
        serverAddress: commonSchemas.serverAddress,
        channelName: { type: 'string', description: 'Voice channel name.' },
        sessionsInfo: {
          type: 'object',
          properties: {
            currentSessionName: commonSchemas.stringProp,
            isLANMatch: commonSchemas.booleanProp,
            maxPlayers: commonSchemas.numberProp,
            currentPlayers: commonSchemas.numberProp,
            localPlayerCount: commonSchemas.numberProp,
            splitScreenEnabled: commonSchemas.booleanProp,
            splitScreenType: commonSchemas.stringProp,
            voiceChatEnabled: commonSchemas.booleanProp,
            activeVoiceChannels: {
              type: 'array',
              items: commonSchemas.stringProp
            },
            isHosting: commonSchemas.booleanProp,
            connectedServerAddress: commonSchemas.stringProp
          },
          description: 'Sessions information (for get_sessions_info).'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_level_structure',
    category: 'world',
    description: 'Create levels and sublevels. Configure World Partition, streaming, data layers, HLOD, and level instances.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_level', 'create_sublevel', 'configure_level_streaming',
            'set_streaming_distance', 'configure_level_bounds',
            'enable_world_partition', 'configure_grid_size', 'create_data_layer',
            'assign_actor_to_data_layer', 'configure_hlod_layer', 'create_minimap_volume',
            'open_level_blueprint', 'add_level_blueprint_node', 'connect_level_blueprint_nodes',
            'create_level_instance', 'create_packed_level_actor',
            'get_level_structure_info'
          ],
          description: 'Level structure action to perform.'
        },
        levelName: commonSchemas.stringProp,
        levelPath: commonSchemas.levelPath,
        parentLevel: commonSchemas.parentLevel,
        templateLevel: commonSchemas.templateLevel,
        bCreateWorldPartition: { type: 'boolean', description: 'Create with World Partition enabled.' },
        sublevelName: commonSchemas.sublevelName,
        sublevelPath: commonSchemas.levelPath,
        streamingMethod: {
          type: 'string',
          enum: ['Blueprint', 'AlwaysLoaded', 'Disabled'],
          description: 'Level streaming method.'
        },
        bShouldBeVisible: { type: 'boolean', description: 'Level should be visible when loaded.' },
        bShouldBlockOnLoad: { type: 'boolean', description: 'Block game until level is loaded.' },
        bDisableDistanceStreaming: { type: 'boolean', description: 'Disable distance-based streaming.' },
        streamingDistance: { type: 'number', description: 'Distance/radius for streaming volume (creates ALevelStreamingVolume).' },
        streamingUsage: {
          type: 'string',
          enum: ['Loading', 'LoadingAndVisibility', 'VisibilityBlockingOnLoad', 'BlockingOnLoad', 'LoadingNotVisible'],
          description: 'Streaming volume usage mode (default: LoadingAndVisibility).'
        },
        createVolume: { type: 'boolean', description: 'Create a streaming volume (true) or just report existing volumes (false). Default: true.' },
        boundsOrigin: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Origin of level bounds.'
        },
        boundsExtent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Extent of level bounds.'
        },
        bAutoCalculateBounds: { type: 'boolean', description: 'Auto-calculate bounds from content.' },
        bEnableWorldPartition: { type: 'boolean', description: 'Enable World Partition for level.' },
        gridCellSize: { type: 'number', description: 'World Partition grid cell size.' },
        loadingRange: { type: 'number', description: 'Loading range for grid cells.' },
        dataLayerName: commonSchemas.dataLayerName,
        dataLayerLabel: { type: 'string', description: 'Display label for the data layer.' },
        bIsInitiallyVisible: { type: 'boolean', description: 'Data layer initially visible.' },
        bIsInitiallyLoaded: { type: 'boolean', description: 'Data layer initially loaded.' },
        dataLayerType: {
          type: 'string',
          enum: ['Runtime', 'Editor'],
          description: 'Type of data layer.'
        },
        actorName: commonSchemas.actorName,
        actorPath: commonSchemas.actorPath,
        hlodLayerName: { type: 'string', description: 'Name of the HLOD layer.' },
        hlodLayerPath: commonSchemas.hlodLayerPath,
        bIsSpatiallyLoaded: { type: 'boolean', description: 'HLOD is spatially loaded.' },
        cellSize: { type: 'number', description: 'HLOD cell size.' },
        loadingDistance: { type: 'number', description: 'HLOD loading distance.' },
        volumeName: commonSchemas.volumeName,
        volumeLocation: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Location of the volume.'
        },
        volumeExtent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Extent of the volume.'
        },
        nodeClass: commonSchemas.nodeClass,
        nodePosition: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp
          },
          description: 'Position of node in graph.'
        },
        nodeName: commonSchemas.nodeName,
        sourceNodeName: commonSchemas.sourceNode,
        sourcePinName: commonSchemas.sourcePin,
        targetNodeName: commonSchemas.targetNode,
        targetPinName: commonSchemas.targetPin,
        levelInstanceName: commonSchemas.levelInstanceName,
        levelAssetPath: { type: 'string', description: 'Path to the level asset for instancing.' },
        instanceLocation: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Location of the level instance.'
        },
        instanceRotation: {
          type: 'object',
          properties: {
            pitch: commonSchemas.numberProp, yaw: commonSchemas.numberProp, roll: commonSchemas.numberProp
          },
          description: 'Rotation of the level instance.'
        },
        instanceScale: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Scale of the level instance.'
        },
        packedLevelName: { type: 'string', description: 'Name for the packed level actor.' },
        bPackBlueprints: { type: 'boolean', description: 'Include blueprints in packed level.' },
        bPackStaticMeshes: { type: 'boolean', description: 'Include static meshes in packed level.' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        success: commonSchemas.booleanProp,
        message: commonSchemas.stringProp,
        levelPath: commonSchemas.levelPath,
        sublevelPath: commonSchemas.levelPath,
        dataLayerName: { type: 'string', description: 'Name of created data layer.' },
        hlodLayerPath: commonSchemas.hlodLayerPath,
        nodeName: commonSchemas.nodeName,
        levelInstanceName: commonSchemas.levelInstanceName,
        levelStructureInfo: {
          type: 'object',
          properties: {
            currentLevel: commonSchemas.stringProp,
            sublevelCount: commonSchemas.numberProp,
            sublevels: {
              type: 'array',
              items: commonSchemas.stringProp
            },
            worldPartitionEnabled: commonSchemas.booleanProp,
            gridCellSize: commonSchemas.numberProp,
            dataLayers: {
              type: 'array',
              items: commonSchemas.stringProp
            },
            hlodLayers: {
              type: 'array',
              items: commonSchemas.stringProp
            },
            levelInstances: {
              type: 'array',
              items: commonSchemas.stringProp
            }
          },
          description: 'Level structure information (for get_level_structure_info).'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_volumes',
    category: 'world',
    description: 'Create trigger volumes, blocking volumes, physics volumes, audio volumes, and navigation bounds.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_trigger_volume', 'create_trigger_box', 'create_trigger_sphere', 'create_trigger_capsule',
            'create_blocking_volume', 'create_kill_z_volume', 'create_pain_causing_volume', 'create_physics_volume',
            'create_audio_volume', 'create_reverb_volume',
            'create_cull_distance_volume', 'create_precomputed_visibility_volume', 'create_lightmass_importance_volume',
            'create_nav_mesh_bounds_volume', 'create_nav_modifier_volume', 'create_camera_blocking_volume',
            'set_volume_extent', 'set_volume_properties',
            'get_volumes_info'
          ],
          description: 'Volume action to perform'
        },
        volumeName: commonSchemas.volumeName,
        volumePath: commonSchemas.volumePath,
        location: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'World location for the volume.'
        },
        rotation: {
          type: 'object',
          properties: {
            pitch: commonSchemas.numberProp, yaw: commonSchemas.numberProp, roll: commonSchemas.numberProp
          },
          description: 'Rotation of the volume.'
        },
        extent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Extent (half-size) of the volume in each axis.'
        },
        sphereRadius: { type: 'number', description: 'Radius for sphere trigger volumes.' },
        capsuleRadius: { type: 'number', description: 'Radius for capsule trigger volumes.' },
        capsuleHalfHeight: { type: 'number', description: 'Half-height for capsule trigger volumes.' },
        boxExtent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Extent for box trigger volumes.'
        },
        bPainCausing: { type: 'boolean', description: 'Whether the volume causes pain/damage.' },
        damagePerSec: { type: 'number', description: 'Damage per second for pain volumes.' },
        damageType: { type: 'string', description: 'Damage type class path for pain volumes.' },
        bWaterVolume: { type: 'boolean', description: 'Whether this is a water volume.' },
        fluidFriction: { type: 'number', description: 'Fluid friction for physics volumes.' },
        terminalVelocity: { type: 'number', description: 'Terminal velocity in the volume.' },
        priority: commonSchemas.priority,
        bEnabled: { type: 'boolean', description: 'Whether the audio volume is enabled.' },
        reverbEffect: { type: 'string', description: 'Reverb effect asset path.' },
        reverbVolume: { type: 'number', description: 'Volume level for reverb (0.0-1.0).' },
        fadeTime: commonSchemas.fadeTime,
        cullDistances: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              size: { type: 'number', description: 'Object size threshold.' },
              cullDistance: { type: 'number', description: 'Distance at which to cull.' }
            }
          },
          description: 'Array of size/distance pairs for cull distance volumes.'
        },
        areaClass: commonSchemas.areaClass,
        bDynamicModifier: { type: 'boolean', description: 'Whether nav modifier updates dynamically.' },
        bUnbound: { type: 'boolean', description: 'Whether post process volume affects entire world.' },
        blendRadius: { type: 'number', description: 'Blend radius for post process volume.' },
        blendWeight: { type: 'number', description: 'Blend weight (0.0-1.0) for post process.' },
        properties: {
          type: 'object',
          description: 'Additional volume-specific properties as key-value pairs.'
        },
        filter: commonSchemas.filter,
        volumeType: { type: 'string', description: 'Type filter for get_volumes_info (e.g., "Trigger", "Physics").' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        success: commonSchemas.booleanProp,
        message: commonSchemas.stringProp,
        volumeName: { type: 'string', description: 'Name of created/modified volume.' },
        volumePath: commonSchemas.volumePath,
        volumeClass: { type: 'string', description: 'Class of the volume.' },
        volumesInfo: {
          type: 'object',
          properties: {
            totalCount: commonSchemas.numberProp,
            volumes: {
              type: 'array',
              items: {
                type: 'object',
                properties: {
                  name: commonSchemas.stringProp,
                  class: commonSchemas.stringProp,
                  location: {
                    type: 'object',
                    properties: {
                      x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
                    }
                  },
                  extent: {
                    type: 'object',
                    properties: {
                      x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
                    }
                  }
                }
              }
            }
          },
          description: 'Volume information (for get_volumes_info).'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_navigation',
    category: 'gameplay',
    description: 'Configure NavMesh settings, add nav modifiers, create nav links and smart links for pathfinding.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'configure_nav_mesh_settings', 'set_nav_agent_properties', 'rebuild_navigation',
            'create_nav_modifier_component', 'set_nav_area_class', 'configure_nav_area_cost',
            'create_nav_link_proxy', 'configure_nav_link', 'set_nav_link_type',
            'create_smart_link', 'configure_smart_link_behavior',
            'get_navigation_info'
          ],
          description: 'Navigation action to perform'
        },
        navMeshPath: { type: 'string', description: 'Path to NavMesh data asset.' },
        actorName: commonSchemas.actorName,
        actorPath: commonSchemas.actorPath,
        blueprintPath: commonSchemas.blueprintPath,
        agentRadius: { type: 'number', description: 'Navigation agent radius (default: 35).' },
        agentHeight: { type: 'number', description: 'Navigation agent height (default: 144).' },
        agentStepHeight: { type: 'number', description: 'Maximum step height agent can climb (default: 35).' },
        agentMaxSlope: { type: 'number', description: 'Maximum slope angle in degrees (default: 44).' },
        cellSize: { type: 'number', description: 'NavMesh cell size (default: 19).' },
        cellHeight: { type: 'number', description: 'NavMesh cell height (default: 10).' },
        tileSizeUU: { type: 'number', description: 'NavMesh tile size in UU (default: 1000).' },
        minRegionArea: { type: 'number', description: 'Minimum region area to keep.' },
        mergeRegionSize: { type: 'number', description: 'Region merge threshold.' },
        maxSimplificationError: { type: 'number', description: 'Edge simplification error.' },
        componentName: commonSchemas.componentName,
        areaClass: commonSchemas.areaClass,
        areaClassToReplace: { type: 'string', description: 'Area class to replace (optional modifier behavior).' },
        failsafeExtent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Failsafe extent for nav modifier when actor has no collision.'
        },
        bIncludeAgentHeight: { type: 'boolean', description: 'Expand lower bounds by agent height.' },
        areaCost: { type: 'number', description: 'Pathfinding cost multiplier for area (1.0 = normal).' },
        fixedAreaEnteringCost: { type: 'number', description: 'Fixed cost added when entering the area.' },
        linkName: commonSchemas.linkName,
        startPoint: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Start point of navigation link (relative to actor).'
        },
        endPoint: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'End point of navigation link (relative to actor).'
        },
        direction: {
          type: 'string',
          enum: ['BothWays', 'LeftToRight', 'RightToLeft'],
          description: 'Link traversal direction.'
        },
        snapRadius: { type: 'number', description: 'Snap radius for link endpoints (default: 30).' },
        linkEnabled: { type: 'boolean', description: 'Whether the link is enabled.' },
        linkType: {
          type: 'string',
          enum: ['simple', 'smart'],
          description: 'Type of navigation link.'
        },
        bSmartLinkIsRelevant: { type: 'boolean', description: 'Toggle smart link relevancy.' },
        enabledAreaClass: { type: 'string', description: 'Area class when smart link is enabled.' },
        disabledAreaClass: { type: 'string', description: 'Area class when smart link is disabled.' },
        broadcastRadius: { type: 'number', description: 'Radius for state change broadcast.' },
        broadcastInterval: { type: 'number', description: 'Interval for state change broadcast (0 = single).' },
        bCreateBoxObstacle: { type: 'boolean', description: 'Add box obstacle during nav generation.' },
        obstacleOffset: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Offset of simple box obstacle.'
        },
        obstacleExtent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Extent of simple box obstacle.'
        },
        obstacleAreaClass: { type: 'string', description: 'Area class for box obstacle.' },
        location: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'World location for nav link proxy.'
        },
        rotation: {
          type: 'object',
          properties: {
            pitch: commonSchemas.numberProp, yaw: commonSchemas.numberProp, roll: commonSchemas.numberProp
          },
          description: 'Rotation for nav link proxy.'
        },
        filter: commonSchemas.filter,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        success: commonSchemas.booleanProp,
        message: commonSchemas.stringProp,
        actorName: commonSchemas.actorName,
        componentName: commonSchemas.componentName,
        navMeshInfo: {
          type: 'object',
          properties: {
            agentRadius: commonSchemas.numberProp,
            agentHeight: commonSchemas.numberProp,
            agentStepHeight: commonSchemas.numberProp,
            agentMaxSlope: commonSchemas.numberProp,
            cellSize: commonSchemas.numberProp,
            cellHeight: commonSchemas.numberProp,
            tileSizeUU: commonSchemas.numberProp,
            tileCount: commonSchemas.numberProp,
            boundsVolumes: commonSchemas.numberProp,
            navLinkCount: commonSchemas.numberProp
          },
          description: 'Navigation system information.'
        },
        error: commonSchemas.stringProp
      }
    }
  },
  {
    name: 'manage_splines',
    category: 'world',
    description: 'Create spline actors, add/modify points, attach meshes along splines, and query spline data.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_spline_actor', 'add_spline_point', 'remove_spline_point',
            'set_spline_point_position', 'set_spline_point_tangents',
            'set_spline_point_rotation', 'set_spline_point_scale', 'set_spline_type',
            'create_spline_mesh_component', 'set_spline_mesh_asset',
            'configure_spline_mesh_axis', 'set_spline_mesh_material',
            'scatter_meshes_along_spline', 'configure_mesh_spacing', 'configure_mesh_randomization',
            'create_road_spline', 'create_river_spline', 'create_fence_spline',
            'create_wall_spline', 'create_cable_spline', 'create_pipe_spline',
            'get_splines_info'
          ],
          description: 'Spline action to perform'
        },
        actorName: commonSchemas.actorName,
        actorPath: commonSchemas.actorPath,
        splineName: { type: 'string', description: 'Name of spline component.' },
        componentName: commonSchemas.componentName,
        blueprintPath: commonSchemas.blueprintPath,
        location: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Location for spline actor.'
        },
        rotation: {
          type: 'object',
          properties: {
            pitch: commonSchemas.numberProp, yaw: commonSchemas.numberProp, roll: commonSchemas.numberProp
          },
          description: 'Rotation for spline actor.'
        },
        scale: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Scale for spline actor.'
        },
        pointIndex: { type: 'number', description: 'Index of spline point to modify.' },
        position: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Position for spline point.'
        },
        arriveTangent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Arrive tangent for spline point (incoming direction).'
        },
        leaveTangent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Leave tangent for spline point (outgoing direction).'
        },
        tangent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Unified tangent (sets both arrive and leave).'
        },
        pointRotation: {
          type: 'object',
          properties: {
            pitch: commonSchemas.numberProp, yaw: commonSchemas.numberProp, roll: commonSchemas.numberProp
          },
          description: 'Rotation at spline point.'
        },
        pointScale: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Scale at spline point.'
        },
        coordinateSpace: {
          type: 'string',
          enum: ['Local', 'World'],
          description: 'Coordinate space for position/tangent values (default: Local).'
        },
        splineType: {
          type: 'string',
          enum: ['Linear', 'Curve', 'Constant', 'CurveClamped', 'CurveCustomTangent'],
          description: 'Type of spline interpolation.'
        },
        bClosedLoop: { type: 'boolean', description: 'Whether spline forms a closed loop.' },
        bUpdateSpline: { type: 'boolean', description: 'Update spline after modification (default: true).' },
        meshPath: commonSchemas.meshPath,
        materialPath: commonSchemas.materialPath,
        forwardAxis: {
          type: 'string',
          enum: ['X', 'Y', 'Z'],
          description: 'Forward axis for spline mesh deformation.'
        },
        startPos: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Start position for spline mesh segment.'
        },
        startTangent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'Start tangent for spline mesh segment.'
        },
        endPos: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'End position for spline mesh segment.'
        },
        endTangent: {
          type: 'object',
          properties: {
            x: commonSchemas.numberProp, y: commonSchemas.numberProp, z: commonSchemas.numberProp
          },
          description: 'End tangent for spline mesh segment.'
        },
        startScale: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'X/Y scale at spline mesh start.'
        },
        endScale: {
          type: 'object',
          properties: commonSchemas.vector2.properties,
          description: 'X/Y scale at spline mesh end.'
        },
        startRoll: { type: 'number', description: 'Roll angle at spline mesh start (radians).' },
        endRoll: { type: 'number', description: 'Roll angle at spline mesh end (radians).' },
        bSmoothInterpRollScale: { type: 'boolean', description: 'Use smooth interpolation for roll/scale.' },
        spacing: { type: 'number', description: 'Distance between scattered meshes.' },
        startOffset: { type: 'number', description: 'Offset from spline start for first mesh.' },
        endOffset: { type: 'number', description: 'Offset from spline end for last mesh.' },
        bAlignToSpline: { type: 'boolean', description: 'Align scattered meshes to spline direction.' },
        bRandomizeRotation: { type: 'boolean', description: 'Apply random rotation to scattered meshes.' },
        rotationRandomRange: {
          type: 'object',
          properties: {
            pitch: commonSchemas.numberProp, yaw: commonSchemas.numberProp, roll: commonSchemas.numberProp
          },
          description: 'Random rotation range (degrees).'
        },
        bRandomizeScale: { type: 'boolean', description: 'Apply random scale to scattered meshes.' },
        scaleMin: { type: 'number', description: 'Minimum random scale multiplier.' },
        scaleMax: { type: 'number', description: 'Maximum random scale multiplier.' },
        randomSeed: { type: 'number', description: 'Seed for randomization (for reproducible results).' },
        templateType: {
          type: 'string',
          enum: ['road', 'river', 'fence', 'wall', 'cable', 'pipe'],
          description: 'Type of spline template to create.'
        },
        width: commonSchemas.width,
        segmentLength: { type: 'number', description: 'Length of mesh segments for deformation.' },
        postSpacing: { type: 'number', description: 'Spacing between fence posts.' },
        railHeight: { type: 'number', description: 'Height of fence rails.' },
        pipeRadius: { type: 'number', description: 'Radius for pipe template.' },
        cableSlack: { type: 'number', description: 'Slack/sag amount for cable template.' },
        points: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              position: {
                type: 'object',
                properties: commonSchemas.vector3.properties
              },
              arriveTangent: {
                type: 'object',
                properties: commonSchemas.vector3.properties
              },
              leaveTangent: {
                type: 'object',
                properties: commonSchemas.vector3.properties
              },
              rotation: {
                type: 'object',
                properties: commonSchemas.rotation.properties
              },
              scale: {
                type: 'object',
                properties: commonSchemas.vector3.properties
              },
              type: {
                type: 'string',
                enum: ['Linear', 'Curve', 'Constant', 'CurveClamped', 'CurveCustomTangent']
              }
            },
            required: ['position']
          },
          description: 'Array of spline points for batch creation.'
        },
        filter: commonSchemas.filter,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        success: commonSchemas.booleanProp,
        message: commonSchemas.stringProp,
        actorName: commonSchemas.actorName,
        componentName: commonSchemas.componentName,
        pointCount: { type: 'number', description: 'Number of points in spline.' },
        splineLength: { type: 'number', description: 'Total length of spline in units.' },
        bClosedLoop: { type: 'boolean', description: 'Whether spline is a closed loop.' },
        splineInfo: {
          type: 'object',
          properties: {
            pointCount: commonSchemas.numberProp,
            splineLength: commonSchemas.numberProp,
            bClosedLoop: commonSchemas.booleanProp,
            points: {
              type: 'array',
              items: {
                type: 'object',
                properties: {
                  index: commonSchemas.numberProp,
                  position: commonSchemas.objectProp,
                  arriveTangent: commonSchemas.objectProp,
                  leaveTangent: commonSchemas.objectProp,
                  rotation: commonSchemas.objectProp,
                  scale: commonSchemas.objectProp,
                  type: commonSchemas.stringProp
                }
              }
            }
          },
          description: 'Detailed spline information.'
        },
        meshComponents: {
          type: 'array',
          items: {
            type: 'object',
            properties: {
              name: commonSchemas.stringProp,
              meshPath: commonSchemas.stringProp,
              forwardAxis: commonSchemas.stringProp
            }
          },
          description: 'List of spline mesh components.'
        },
        scatteredMeshes: { type: 'number', description: 'Number of meshes scattered along spline.' },
        error: commonSchemas.stringProp
      }
    }
  },

  // ============================================
  // FORGE2: WORLD GENERATION TOOLS
  // ============================================

  {
    name: 'manage_worldgen',
    category: 'world',
    description: 'Procedural world generation using native UE systems. Generate terrain with noise algorithms, apply erosion, define biomes, create water bodies, generate buildings/dungeons/cities with full interiors, and populate environments.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'generate_heightmap', 'generate_noise_terrain', 'apply_thermal_erosion', 'apply_hydraulic_erosion',
            'import_heightmap', 'export_heightmap', 'define_biome', 'create_biome_map', 'apply_biome', 'blend_biome_edges',
            'create_river', 'create_lake', 'create_ocean', 'simulate_water_flow',
            'generate_building', 'generate_dungeon', 'generate_city', 'generate_village',
            'generate_road_network', 'generate_bridge', 'generate_wall', 'generate_tower',
            'generate_interior', 'furnish_room',
            'create_pcg_graph', 'add_pcg_node', 'connect_pcg_nodes', 'execute_pcg_graph', 'set_pcg_settings',
            'scatter_props', 'place_vegetation', 'generate_rocks', 'place_decals'
          ],
          description: 'World generation action'
        },
        // Terrain generation
        noiseType: { type: 'string', enum: ['perlin', 'simplex', 'voronoi', 'fbm', 'ridged', 'worley'], description: 'Noise algorithm type.' },
        seed: commonSchemas.seed,
        scale: commonSchemas.noiseScale,
        octaves: commonSchemas.octaves,
        persistence: commonSchemas.persistence,
        lacunarity: commonSchemas.lacunarity,
        amplitude: commonSchemas.amplitude,
        frequency: commonSchemas.frequency,
        // Erosion
        erosionIterations: commonSchemas.erosionIterations,
        erosionStrength: commonSchemas.erosionStrength,
        sedimentCapacity: commonSchemas.sedimentCapacity,
        evaporationRate: commonSchemas.evaporationRate,
        // Biome
        biomeName: commonSchemas.biomeName,
        temperatureRange: commonSchemas.temperatureRange,
        moistureRange: commonSchemas.moistureRange,
        elevationRange: commonSchemas.elevationRange,
        vegetationDensity: commonSchemas.vegetationDensity,
        materialPath: commonSchemas.materialPath,
        // Structure generation
        buildingStyle: commonSchemas.buildingStyle,
        buildingSize: commonSchemas.buildingSize,
        floorCount: commonSchemas.floorCount,
        includeInterior: commonSchemas.includeInterior,
        interiorStyle: commonSchemas.stringProp,
        furnitureDensity: commonSchemas.furnitureDensity,
        // Dungeon
        dungeonAlgorithm: commonSchemas.dungeonAlgorithm,
        roomCount: commonSchemas.roomCount,
        roomSizeRange: commonSchemas.roomSizeRange,
        corridorWidth: commonSchemas.corridorWidth,
        includeLoops: commonSchemas.includeLoops,
        roomTypes: commonSchemas.roomTypes,
        // City
        districtTypes: commonSchemas.districtTypes,
        buildingDensity: commonSchemas.buildingDensity,
        roadStyle: commonSchemas.roadStyle,
        includeWalls: commonSchemas.includeWalls,
        populationTarget: commonSchemas.populationTarget,
        // Water
        waterType: commonSchemas.waterType,
        flowRate: commonSchemas.flowRate,
        depth: commonSchemas.depth,
        waveIntensity: commonSchemas.waveIntensity,
        // PCG
        pcgGraphPath: commonSchemas.assetPath,
        pcgNodeType: commonSchemas.stringProp,
        pcgSettings: commonSchemas.objectProp,
        // Population
        density: commonSchemas.numberProp,
        randomRotation: commonSchemas.booleanProp,
        randomScaleRange: commonSchemas.arrayOfNumbers,
        alignToSurface: commonSchemas.booleanProp,
        meshes: commonSchemas.arrayOfStrings,
        // Common
        location: commonSchemas.location,
        bounds: commonSchemas.bounds,
        outputPath: commonSchemas.outputPath,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        generatedPath: commonSchemas.assetPath,
        actorName: commonSchemas.actorName,
        biomeId: commonSchemas.stringProp,
        dungeonData: commonSchemas.objectProp,
        cityData: commonSchemas.objectProp,
        pcgGraphId: commonSchemas.stringProp
      }
    }
  },

  // ============================================
  // FORGE2: RPG SYSTEM TOOLS
  // ============================================

  {
    name: 'manage_quest',
    category: 'gameplay',
    description: 'Quest system management. Create quests with objectives, rewards, prerequisites, chains, and branching. Configure markers and failure conditions.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_quest', 'add_objective', 'set_rewards', 'add_prerequisite',
            'create_quest_chain', 'add_branch', 'set_dialogue_trigger', 'configure_markers',
            'set_fail_conditions', 'create_optional_objective', 'add_timed_objective', 'set_auto_track',
            'get_quest', 'list_quests', 'delete_quest', 'update_quest'
          ],
          description: 'Quest action'
        },
        questId: commonSchemas.questId,
        questName: commonSchemas.questName,
        description: commonSchemas.stringProp,
        category: commonSchemas.questCategory,
        levelRequirement: commonSchemas.levelRequirement,
        repeatable: commonSchemas.repeatable,
        // Objectives
        objectiveId: commonSchemas.objectiveId,
        objectiveType: commonSchemas.objectiveType,
        target: commonSchemas.stringProp,
        targetCount: commonSchemas.targetCount,
        targetLocation: commonSchemas.location,
        timeLimit: commonSchemas.timeLimit,
        optional: commonSchemas.isOptional,
        hiddenUntilProgress: commonSchemas.numberProp,
        // Rewards
        xpReward: commonSchemas.xpReward,
        currencyRewards: commonSchemas.currencyRewards,
        itemRewards: commonSchemas.itemRewards,
        reputationRewards: commonSchemas.reputationRewards,
        abilityUnlock: commonSchemas.stringProp,
        titleUnlock: commonSchemas.stringProp,
        // Prerequisites
        requiredQuests: commonSchemas.requiredQuests,
        requiredLevel: commonSchemas.levelRequirement,
        requiredReputation: commonSchemas.requiredReputation,
        requiredItems: commonSchemas.arrayOfStrings,
        requiredClass: commonSchemas.stringProp,
        // Chain/branching
        chainId: commonSchemas.chainId,
        nextQuest: commonSchemas.nextQuest,
        branchConditions: commonSchemas.branchConditions,
        // Markers
        showCompassMarker: commonSchemas.showCompassMarker,
        showMapMarker: commonSchemas.showMapMarker,
        markerIcon: commonSchemas.markerIcon,
        trackingDistance: commonSchemas.trackingDistance,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        questId: commonSchemas.questId,
        questPath: commonSchemas.assetPath,
        objectiveId: commonSchemas.objectiveId,
        chainId: commonSchemas.chainId
      }
    }
  },

  {
    name: 'manage_dialogue',
    category: 'gameplay',
    description: 'Dialogue tree management. Create branching conversations with speaker profiles, conditions, consequences, and quest integration.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_dialogue_tree', 'add_dialogue_node', 'connect_dialogue_nodes',
            'create_speaker_profile', 'add_choice_condition', 'add_consequence',
            'set_dialogue_trigger', 'configure_audio', 'configure_animation', 'configure_camera',
            'link_to_quest', 'get_dialogue', 'list_dialogues', 'delete_dialogue'
          ],
          description: 'Dialogue action'
        },
        dialogueTreeId: commonSchemas.dialogueTreeId,
        dialogueNodeId: commonSchemas.dialogueNodeId,
        speakerName: commonSchemas.speakerName,
        dialogueText: commonSchemas.dialogueText,
        portraitPath: commonSchemas.portraitPath,
        voicePath: commonSchemas.voicePath,
        animationPath: commonSchemas.animationPath,
        // Node connections
        fromNodeId: commonSchemas.sourceNodeId,
        toNodeId: commonSchemas.targetNodeId,
        // Conditions and consequences
        conditionType: { type: 'string', enum: ['has_item', 'quest_complete', 'skill_check', 'reputation_check', 'stat_check', 'custom'], description: 'Condition type.' },
        conditionParams: commonSchemas.objectProp,
        consequenceType: { type: 'string', enum: ['set_variable', 'give_item', 'take_item', 'start_quest', 'complete_objective', 'change_reputation', 'trigger_event'], description: 'Consequence type.' },
        consequenceParams: commonSchemas.objectProp,
        // Quest link
        questId: commonSchemas.questId,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        dialogueTreeId: commonSchemas.dialogueTreeId,
        dialoguePath: commonSchemas.assetPath,
        nodeId: commonSchemas.dialogueNodeId,
        speakerId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_skill_tree',
    category: 'gameplay',
    description: 'Skill tree and class management. Create skill trees with nodes, define skills (passive, active, toggle, aura, ultimate), configure prerequisites, and manage class/subclass definitions.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_skill_tree', 'add_skill_node', 'connect_skill_nodes',
            'create_skill', 'add_skill_prerequisite', 'configure_skill_scaling',
            'create_class', 'configure_subclass', 'setup_multiclass',
            'configure_respec', 'get_skill_tree', 'list_skill_trees', 'delete_skill_tree'
          ],
          description: 'Skill tree action'
        },
        skillTreeId: commonSchemas.skillTreeId,
        skillId: commonSchemas.skillId,
        skillName: commonSchemas.skillName,
        description: commonSchemas.stringProp,
        skillType: commonSchemas.skillType,
        ranks: commonSchemas.skillRanks,
        pointCost: commonSchemas.pointCost,
        prerequisites: commonSchemas.skillPrerequisites,
        // Node positioning
        nodeX: commonSchemas.nodeX,
        nodeY: commonSchemas.nodeY,
        // Scaling
        scalingPerRank: commonSchemas.objectProp,
        statRequirements: commonSchemas.objectProp,
        // Class
        classId: commonSchemas.classId,
        className: commonSchemas.className,
        baseStats: commonSchemas.baseStats,
        startingAbilities: commonSchemas.startingAbilities,
        equipmentRestrictions: commonSchemas.equipmentRestrictions,
        // Subclass
        specializationBonuses: commonSchemas.objectProp,
        // Respec
        respecCost: commonSchemas.numberProp,
        partialRespec: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        skillTreeId: commonSchemas.skillTreeId,
        skillTreePath: commonSchemas.assetPath,
        skillId: commonSchemas.skillId,
        classId: commonSchemas.classId
      }
    }
  },

  {
    name: 'manage_economy',
    category: 'gameplay',
    description: 'Economy system management. Create currencies, shops, trading, auction house, and configure gold sinks.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_currency', 'create_shop', 'add_shop_item', 'configure_dynamic_stock',
            'create_trading_system', 'configure_trade_tax', 'create_auction_house',
            'configure_listing_fee', 'configure_gold_sinks', 'configure_dynamic_pricing',
            'get_currency', 'list_currencies', 'get_shop', 'list_shops'
          ],
          description: 'Economy action'
        },
        currencyId: commonSchemas.currencyId,
        currencyName: commonSchemas.currencyName,
        currencyType: commonSchemas.currencyType,
        currencyCap: commonSchemas.currencyCap,
        currencyIcon: commonSchemas.iconPath,
        // Shop
        shopId: commonSchemas.shopId,
        shopName: commonSchemas.stringProp,
        shopInventory: commonSchemas.shopInventory,
        dynamicStock: commonSchemas.dynamicStock,
        stockRefreshTime: commonSchemas.stockRefreshTime,
        reputationPricing: commonSchemas.reputationPricing,
        buySellRatio: commonSchemas.numberProp,
        // Trading
        tradeTax: commonSchemas.numberProp,
        // Auction
        listingFee: commonSchemas.listingFee,
        listingDuration: commonSchemas.listingDuration,
        biddingEnabled: commonSchemas.biddingEnabled,
        buyoutEnabled: commonSchemas.buyoutEnabled,
        // Gold sinks
        repairCosts: commonSchemas.objectProp,
        travelCosts: commonSchemas.objectProp,
        respecCosts: commonSchemas.objectProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        currencyId: commonSchemas.currencyId,
        currencyPath: commonSchemas.assetPath,
        shopId: commonSchemas.shopId,
        shopPath: commonSchemas.assetPath
      }
    }
  },

  {
    name: 'manage_progression',
    category: 'gameplay',
    description: 'Progression system management. Configure XP curves, level rewards, XP sources, achievements, factions, and leaderboards.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'configure_xp_curve', 'configure_level_rewards', 'configure_xp_sources', 'configure_xp_modifiers',
            'create_achievement', 'configure_achievement_criteria', 'setup_progress_tracking',
            'create_faction', 'configure_faction_ranks', 'configure_faction_rewards', 'configure_faction_relationships',
            'create_leaderboard', 'configure_seasonal_resets', 'configure_statistics_tracking',
            'get_progression_config', 'list_achievements', 'list_factions'
          ],
          description: 'Progression action'
        },
        // XP curve
        curveType: { type: 'string', enum: ['linear', 'exponential', 'polynomial', 'custom'], description: 'XP curve type.' },
        baseXP: commonSchemas.numberProp,
        scalingFactor: commonSchemas.numberProp,
        customCurve: commonSchemas.arrayOfNumbers,
        // Level rewards
        levelRewards: commonSchemas.objectProp,
        skillPointsPerLevel: commonSchemas.integerProp,
        statIncreasePerLevel: commonSchemas.objectProp,
        // XP sources
        killXPBase: commonSchemas.numberProp,
        questXPBase: commonSchemas.numberProp,
        explorationXPBase: commonSchemas.numberProp,
        craftingXPBase: commonSchemas.numberProp,
        // Achievement
        achievementId: commonSchemas.stringProp,
        achievementName: commonSchemas.stringProp,
        achievementDescription: commonSchemas.stringProp,
        achievementIcon: commonSchemas.iconPath,
        achievementPoints: commonSchemas.integerProp,
        criteriaType: commonSchemas.stringProp,
        criteriaTarget: commonSchemas.objectProp,
        isSecret: commonSchemas.booleanProp,
        // Faction
        factionId: commonSchemas.stringProp,
        factionName: commonSchemas.stringProp,
        factionRanks: commonSchemas.arrayOfObjects,
        rankThresholds: commonSchemas.arrayOfNumbers,
        rankRewards: commonSchemas.arrayOfObjects,
        factionRelationships: commonSchemas.objectProp,
        // Leaderboard
        leaderboardId: commonSchemas.stringProp,
        seasonalReset: commonSchemas.booleanProp,
        resetInterval: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        achievementId: commonSchemas.stringProp,
        achievementPath: commonSchemas.assetPath,
        factionId: commonSchemas.stringProp,
        factionPath: commonSchemas.assetPath,
        leaderboardId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_combat_stats',
    category: 'gameplay',
    description: 'Combat statistics and status effect management. Define stats, derived stats, damage types, resistances, status effects, and damage formulas.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_stat', 'create_derived_stat', 'configure_stat_scaling',
            'create_damage_type', 'configure_resistance_system',
            'create_status_effect', 'configure_effect_stacking', 'configure_effect_visuals',
            'configure_damage_formula', 'configure_healing_formula',
            'create_combat_profile', 'configure_ai_targeting', 'configure_ai_ability_usage',
            'get_stat', 'list_stats', 'get_damage_type', 'list_damage_types', 'get_effect', 'list_effects'
          ],
          description: 'Combat stats action'
        },
        // Stats
        statId: commonSchemas.statId,
        statName: commonSchemas.statName,
        baseValue: commonSchemas.baseValue,
        minValue: commonSchemas.minValue,
        maxValue: commonSchemas.maxValue,
        derivedFormula: commonSchemas.derivedFormula,
        scalingPerLevel: commonSchemas.numberProp,
        // Damage types
        damageTypeId: commonSchemas.stringProp,
        damageTypeName: commonSchemas.stringProp,
        resistanceStat: commonSchemas.stringProp,
        // Status effects
        effectId: commonSchemas.effectId,
        effectName: commonSchemas.effectName,
        effectType: commonSchemas.effectType,
        duration: commonSchemas.effectDuration,
        stackable: commonSchemas.stackable,
        maxStacks: commonSchemas.maxStacks,
        tickInterval: commonSchemas.tickInterval,
        effectValue: commonSchemas.numberProp,
        effectIcon: commonSchemas.iconPath,
        effectVFX: commonSchemas.niagaraPath,
        // Formulas
        damageFormula: commonSchemas.damageFormula,
        criticalChance: commonSchemas.criticalChance,
        criticalDamage: commonSchemas.criticalDamage,
        // Combat profile
        profileId: commonSchemas.stringProp,
        aggressionLevel: commonSchemas.numberProp,
        preferredAbilities: commonSchemas.arrayOfStrings,
        targetingPriority: commonSchemas.stringProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        statId: commonSchemas.statId,
        statPath: commonSchemas.assetPath,
        damageTypeId: commonSchemas.stringProp,
        effectId: commonSchemas.effectId,
        effectPath: commonSchemas.assetPath,
        profileId: commonSchemas.stringProp
      }
    }
  },

  // ============================================
  // FORGE2: CONTEXT ENGINEERING TOOLS
  // ============================================

  {
    name: 'manage_context',
    category: 'core',
    description: 'Context engineering for AI-assisted development. Manage project state, todo lists, decisions, and session data using file-based memory.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'get_project_summary', 'get_category_index', 'drill_down',
            'update_todo', 'add_todo', 'complete_todo', 'remove_todo', 'list_todos',
            'log_decision', 'get_decisions', 'clear_decisions',
            'save_session', 'load_session', 'clear_session',
            'get_asset_index', 'refresh_index'
          ],
          description: 'Context action'
        },
        // Todo
        todoItem: commonSchemas.todoItem,
        todoId: commonSchemas.stringProp,
        todoStatus: commonSchemas.todoStatus,
        todoPriority: commonSchemas.integerProp,
        // Decision
        decisionText: commonSchemas.decisionText,
        decisionReason: commonSchemas.decisionReason,
        decisionCategory: commonSchemas.stringProp,
        // Session
        sessionData: commonSchemas.sessionData,
        sessionKey: commonSchemas.stringProp,
        // Navigation
        path: commonSchemas.assetPath,
        category: commonSchemas.stringProp,
        maxTokens: commonSchemas.integerProp
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        projectSummary: commonSchemas.objectProp,
        todos: commonSchemas.arrayOfObjects,
        decisions: commonSchemas.arrayOfObjects,
        sessionData: commonSchemas.objectProp,
        assetIndex: commonSchemas.objectProp,
        categoryData: commonSchemas.objectProp
      }
    }
  },

  // ============================================
  // FORGE2: GENRE-SPECIFIC TOOLS
  // ============================================

  {
    name: 'manage_genre_racing',
    category: 'genre',
    description: 'Racing/driving game mechanics. Create vehicles, configure physics (arcade/simulation), build tracks, setup race modes, configure tire/damage systems, and AI racers.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_vehicle', 'configure_vehicle_physics', 'configure_tire_system', 'setup_damage_system',
            'create_track', 'add_checkpoint', 'configure_track_boundaries',
            'setup_race_mode', 'configure_race_rules',
            'create_powerup_system', 'add_powerup',
            'configure_ai_racers', 'setup_rubber_banding',
            'setup_leaderboards', 'record_ghost_data',
            'get_vehicle', 'list_vehicles', 'get_track', 'list_tracks'
          ],
          description: 'Racing action'
        },
        // Vehicle
        vehicleId: { type: 'string', description: 'Unique vehicle identifier.' },
        vehicleName: commonSchemas.stringProp,
        vehicleType: { type: 'string', enum: ['car', 'bike', 'boat', 'aircraft', 'kart', 'truck'], description: 'Vehicle type.' },
        meshPath: commonSchemas.meshPath,
        // Physics
        handlingPreset: { type: 'string', enum: ['arcade', 'simulation', 'semi_realistic'], description: 'Handling style.' },
        acceleration: commonSchemas.numberProp,
        topSpeed: commonSchemas.numberProp,
        brakingForce: commonSchemas.numberProp,
        turningRadius: commonSchemas.numberProp,
        weightDistribution: commonSchemas.objectProp,
        suspensionSettings: commonSchemas.objectProp,
        // Tires
        tireGrip: commonSchemas.numberProp,
        tireWear: commonSchemas.booleanProp,
        tireTemperature: commonSchemas.booleanProp,
        tireTypes: { type: 'array', items: { type: 'string', enum: ['soft', 'medium', 'hard', 'wet', 'offroad'] }, description: 'Available tire types.' },
        // Damage
        visualDamage: commonSchemas.booleanProp,
        mechanicalDamage: commonSchemas.booleanProp,
        componentDamage: commonSchemas.objectProp,
        // Track
        trackId: { type: 'string', description: 'Unique track identifier.' },
        trackName: commonSchemas.stringProp,
        trackLength: commonSchemas.numberProp,
        checkpoints: { type: 'array', items: commonSchemas.objectProp, description: 'Checkpoint positions.' },
        pitLane: commonSchemas.booleanProp,
        // Race mode
        raceMode: { type: 'string', enum: ['time_trial', 'circuit', 'elimination', 'drift', 'drag', 'rally'], description: 'Race mode type.' },
        lapCount: commonSchemas.integerProp,
        aiCount: commonSchemas.integerProp,
        // Powerups
        powerupType: { type: 'string', enum: ['boost', 'weapon', 'shield', 'slow', 'teleport'], description: 'Powerup type.' },
        powerupSpawnLocations: { type: 'array', items: commonSchemas.location, description: 'Spawn positions.' },
        // AI
        aiDifficulty: { type: 'string', enum: ['easy', 'normal', 'hard', 'expert'], description: 'AI difficulty.' },
        rubberBandingEnabled: commonSchemas.booleanProp,
        rubberBandingStrength: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        vehicleId: commonSchemas.stringProp,
        vehiclePath: commonSchemas.assetPath,
        trackId: commonSchemas.stringProp,
        trackPath: commonSchemas.assetPath
      }
    }
  },

  {
    name: 'manage_genre_fighting',
    category: 'genre',
    description: 'Fighting game mechanics. Create fighters with hitboxes/hurtboxes, define movesets, configure combo systems, setup frame data, manage meters, and blocking systems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_fighter', 'configure_hitboxes', 'configure_hurtboxes',
            'define_moveset', 'add_move', 'configure_move_properties',
            'configure_combo_system', 'setup_combo_routes', 'configure_damage_scaling',
            'setup_frame_data', 'configure_advantage',
            'create_special_meter', 'configure_meter_gain',
            'configure_blocking', 'setup_parry_system', 'configure_guard_break',
            'setup_stage', 'configure_stage_hazards', 'configure_wall_mechanics',
            'create_training_mode', 'configure_hitbox_display',
            'get_fighter', 'list_fighters'
          ],
          description: 'Fighting action'
        },
        // Fighter
        fighterId: { type: 'string', description: 'Unique fighter identifier.' },
        fighterName: commonSchemas.stringProp,
        skeletonPath: commonSchemas.skeletalMeshPath,
        // Hitbox/Hurtbox
        hitboxes: { type: 'array', items: commonSchemas.objectProp, description: 'Hitbox definitions.' },
        hurtboxes: { type: 'array', items: commonSchemas.objectProp, description: 'Hurtbox definitions.' },
        // Moveset
        moveId: { type: 'string', description: 'Move identifier.' },
        moveType: { type: 'string', enum: ['normal', 'special', 'super', 'ultra', 'command_normal', 'throw'], description: 'Move type.' },
        moveInput: { type: 'string', description: 'Input sequence (e.g., "236P" for QCF+Punch).' },
        damage: commonSchemas.numberProp,
        // Frame data
        startupFrames: commonSchemas.integerProp,
        activeFrames: commonSchemas.integerProp,
        recoveryFrames: commonSchemas.integerProp,
        blockAdvantage: commonSchemas.integerProp,
        hitAdvantage: commonSchemas.integerProp,
        // Combo
        juggleLimit: commonSchemas.integerProp,
        damageScaling: { type: 'array', items: commonSchemas.numberProp, description: 'Damage scaling per hit.' },
        hitstunDecay: commonSchemas.numberProp,
        comboBreaker: commonSchemas.booleanProp,
        // Meter
        meterType: { type: 'string', enum: ['super', 'rage', 'burst', 'v_gauge', 'drive'], description: 'Meter type.' },
        meterMax: commonSchemas.numberProp,
        meterGainOnHit: commonSchemas.numberProp,
        meterGainOnBlock: commonSchemas.numberProp,
        // Blocking
        blockType: { type: 'string', enum: ['high', 'low', 'mid', 'unblockable'], description: 'Attack block type.' },
        parryWindow: commonSchemas.integerProp,
        guardBreakThreshold: commonSchemas.numberProp,
        chipDamage: commonSchemas.numberProp,
        // Stage
        stageId: { type: 'string', description: 'Stage identifier.' },
        stageBounds: commonSchemas.bounds,
        wallSplat: commonSchemas.booleanProp,
        stageTransitions: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        fighterId: commonSchemas.stringProp,
        fighterPath: commonSchemas.assetPath,
        moveId: commonSchemas.stringProp,
        stageId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_genre_platformer',
    category: 'genre',
    description: 'Platformer game mechanics. Create 2D/3D player controllers, configure jump physics, setup platform types, collectibles, checkpoints, hazards, and powerups.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_player_controller_2d', 'create_player_controller_3d',
            'configure_movement', 'configure_jump_physics',
            'create_platform_types', 'add_platform', 'configure_platform_behavior',
            'setup_collectibles', 'add_collectible', 'configure_collection_tracking',
            'create_checkpoint_system', 'add_checkpoint',
            'configure_hazards', 'add_hazard',
            'setup_powerups', 'add_powerup', 'configure_powerup_duration',
            'get_level_stats', 'validate_level'
          ],
          description: 'Platformer action'
        },
        // Controller
        controllerId: { type: 'string', description: 'Controller identifier.' },
        controllerType: { type: 'string', enum: ['2d_sidescroll', '2d_topdown', '3d_third_person', '3d_first_person'], description: 'Controller type.' },
        // Movement
        runSpeed: commonSchemas.numberProp,
        sprintSpeed: commonSchemas.numberProp,
        acceleration: commonSchemas.numberProp,
        deceleration: commonSchemas.numberProp,
        airControl: commonSchemas.numberProp,
        // Jump physics
        jumpHeight: commonSchemas.numberProp,
        jumpCount: commonSchemas.integerProp,
        coyoteTime: commonSchemas.numberProp,
        jumpBuffering: commonSchemas.numberProp,
        variableJumpHeight: commonSchemas.booleanProp,
        wallJump: commonSchemas.booleanProp,
        wallSlide: commonSchemas.booleanProp,
        dashEnabled: commonSchemas.booleanProp,
        dashDistance: commonSchemas.numberProp,
        // Platform
        platformId: { type: 'string', description: 'Platform identifier.' },
        platformType: { type: 'string', enum: ['static', 'moving', 'falling', 'bouncy', 'ice', 'conveyor', 'one_way', 'crumbling'], description: 'Platform type.' },
        platformPath: { type: 'array', items: commonSchemas.location, description: 'Movement path for moving platforms.' },
        platformSpeed: commonSchemas.numberProp,
        fallDelay: commonSchemas.numberProp,
        bounceForce: commonSchemas.numberProp,
        conveyorDirection: commonSchemas.objectProp,
        // Collectibles
        collectibleType: { type: 'string', enum: ['coin', 'gem', 'star', 'key', 'heart', 'custom'], description: 'Collectible type.' },
        collectibleValue: commonSchemas.integerProp,
        totalCollectibles: commonSchemas.integerProp,
        // Checkpoint
        checkpointId: { type: 'string', description: 'Checkpoint identifier.' },
        respawnLocation: commonSchemas.location,
        activationFeedback: commonSchemas.booleanProp,
        // Hazards
        hazardType: { type: 'string', enum: ['spike', 'pit', 'enemy', 'fire', 'water', 'laser', 'crusher'], description: 'Hazard type.' },
        hazardDamage: commonSchemas.numberProp,
        instantKill: commonSchemas.booleanProp,
        // Powerups
        powerupId: { type: 'string', description: 'Powerup identifier.' },
        powerupAbility: { type: 'string', enum: ['double_jump', 'glide', 'dash', 'invincibility', 'size_change', 'speed_boost'], description: 'Powerup ability.' },
        powerupDuration: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        controllerId: commonSchemas.stringProp,
        platformId: commonSchemas.stringProp,
        checkpointId: commonSchemas.stringProp,
        powerupId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_genre_stealth',
    category: 'genre',
    description: 'Stealth game mechanics. Create detection systems, configure AI awareness states, setup patrol paths, distractions, cover systems, disguises, and security systems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_detection_system', 'configure_vision_cone', 'configure_hearing',
            'configure_ai_awareness', 'setup_alert_states', 'configure_search_patterns',
            'setup_patrol_paths', 'add_patrol_route', 'configure_patrol_behavior',
            'create_distraction_system', 'add_distraction_type',
            'configure_cover_system', 'add_cover_spot', 'configure_takedowns',
            'setup_disguise_system', 'add_disguise', 'configure_social_stealth',
            'create_security_systems', 'add_camera', 'add_laser', 'configure_alarms',
            'configure_shadow_system', 'configure_light_detection',
            'get_stealth_config', 'validate_level_stealth'
          ],
          description: 'Stealth action'
        },
        // Detection
        visionConeAngle: commonSchemas.numberProp,
        visionDistance: commonSchemas.numberProp,
        peripheralVision: commonSchemas.booleanProp,
        hearingRadius: commonSchemas.numberProp,
        suspicionRate: commonSchemas.numberProp,
        // AI Awareness
        awarenessState: { type: 'string', enum: ['unaware', 'suspicious', 'alert', 'searching', 'combat'], description: 'AI awareness state.' },
        stateTransitionTime: commonSchemas.numberProp,
        searchDuration: commonSchemas.numberProp,
        searchPattern: { type: 'string', enum: ['expanding', 'last_known', 'random', 'systematic'], description: 'Search pattern.' },
        // Patrol
        patrolId: { type: 'string', description: 'Patrol route identifier.' },
        patrolPoints: { type: 'array', items: commonSchemas.location, description: 'Patrol waypoints.' },
        patrolWaitTimes: { type: 'array', items: commonSchemas.numberProp, description: 'Wait time at each point.' },
        patrolRandomization: commonSchemas.booleanProp,
        // Distraction
        distractionType: { type: 'string', enum: ['noise', 'thrown_object', 'decoy', 'radio', 'alarm_false'], description: 'Distraction type.' },
        distractionRadius: commonSchemas.numberProp,
        distractionDuration: commonSchemas.numberProp,
        // Cover
        coverId: { type: 'string', description: 'Cover spot identifier.' },
        coverType: { type: 'string', enum: ['crouch', 'stand', 'corner', 'prone'], description: 'Cover type.' },
        coverLocation: commonSchemas.location,
        takedownEnabled: commonSchemas.booleanProp,
        takedownType: { type: 'string', enum: ['lethal', 'non_lethal', 'both'], description: 'Takedown type.' },
        // Disguise
        disguiseId: { type: 'string', description: 'Disguise identifier.' },
        disguiseName: commonSchemas.stringProp,
        restrictedAreas: { type: 'array', items: commonSchemas.stringProp, description: 'Areas disguise grants access.' },
        suspicionModifier: commonSchemas.numberProp,
        // Security
        cameraId: { type: 'string', description: 'Camera identifier.' },
        cameraFOV: commonSchemas.numberProp,
        cameraPanRange: commonSchemas.numberProp,
        cameraPanSpeed: commonSchemas.numberProp,
        laserPatterns: { type: 'array', items: commonSchemas.objectProp, description: 'Laser movement patterns.' },
        alarmDelay: commonSchemas.numberProp,
        lockdownEnabled: commonSchemas.booleanProp,
        // Shadow/Light
        shadowThreshold: commonSchemas.numberProp,
        lightLevelDetection: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        detectionSystemId: commonSchemas.stringProp,
        patrolId: commonSchemas.stringProp,
        coverId: commonSchemas.stringProp,
        disguiseId: commonSchemas.stringProp,
        cameraId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_genre_puzzle',
    category: 'genre',
    description: 'Puzzle game mechanics. Create puzzle elements, configure logic systems, setup physics puzzles, sequence puzzles, environmental puzzles, and hint systems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_puzzle_element', 'configure_element_behavior', 'connect_elements',
            'configure_logic_system', 'add_logic_gate', 'configure_gate_inputs',
            'setup_physics_puzzles', 'configure_physics_constraints',
            'create_sequence_puzzles', 'configure_sequence_order',
            'configure_environmental_puzzles', 'setup_perspective_puzzle', 'setup_shadow_puzzle',
            'setup_hint_system', 'add_hint', 'configure_hint_timing',
            'create_puzzle_validation', 'configure_multiple_solutions',
            'get_puzzle', 'list_puzzles', 'test_puzzle_solution'
          ],
          description: 'Puzzle action'
        },
        // Puzzle element
        elementId: { type: 'string', description: 'Puzzle element identifier.' },
        elementType: { type: 'string', enum: ['button', 'switch', 'lever', 'door', 'laser', 'portal', 'box', 'mirror', 'pressure_plate', 'key', 'lock'], description: 'Element type.' },
        elementState: { type: 'string', enum: ['active', 'inactive', 'locked', 'unlocked'], description: 'Initial state.' },
        linkedElements: { type: 'array', items: commonSchemas.stringProp, description: 'Connected element IDs.' },
        // Logic
        gateType: { type: 'string', enum: ['and', 'or', 'not', 'xor', 'nand', 'nor', 'timer', 'counter'], description: 'Logic gate type.' },
        gateInputs: { type: 'array', items: commonSchemas.stringProp, description: 'Input element IDs.' },
        gateOutput: commonSchemas.stringProp,
        timerDuration: commonSchemas.numberProp,
        counterTarget: commonSchemas.integerProp,
        // Physics puzzle
        physicsObjectId: { type: 'string', description: 'Physics object identifier.' },
        mass: commonSchemas.numberProp,
        friction: commonSchemas.numberProp,
        bounciness: commonSchemas.numberProp,
        trajectoryGuide: commonSchemas.booleanProp,
        // Sequence
        sequenceId: { type: 'string', description: 'Sequence puzzle identifier.' },
        correctSequence: { type: 'array', items: commonSchemas.stringProp, description: 'Correct order of elements.' },
        resetOnWrong: commonSchemas.booleanProp,
        // Environmental
        perspectiveAngle: commonSchemas.objectProp,
        shadowCaster: commonSchemas.stringProp,
        reflectionSurface: commonSchemas.stringProp,
        // Hints
        hintId: { type: 'string', description: 'Hint identifier.' },
        hintText: commonSchemas.stringProp,
        hintDelay: commonSchemas.numberProp,
        progressiveHints: commonSchemas.booleanProp,
        skipOption: commonSchemas.booleanProp,
        // Validation
        solutionCount: commonSchemas.integerProp,
        validSolutions: { type: 'array', items: commonSchemas.objectProp, description: 'Valid solution configurations.' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        elementId: commonSchemas.stringProp,
        puzzleId: commonSchemas.stringProp,
        sequenceId: commonSchemas.stringProp,
        isSolved: commonSchemas.booleanProp
      }
    }
  },

  {
    name: 'manage_genre_strategy',
    category: 'genre',
    description: 'Strategy/RTS game mechanics. Create units, buildings, configure resource systems, tech trees, fog of war, formations, and AI opponents.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_unit_type', 'configure_unit_stats', 'configure_unit_abilities',
            'configure_building_system', 'create_building', 'configure_production',
            'setup_resource_system', 'add_resource_type', 'configure_gathering',
            'create_tech_tree', 'add_research', 'configure_upgrades',
            'configure_fog_of_war', 'setup_visibility',
            'setup_formation_system', 'create_formation', 'configure_unit_stances',
            'create_command_system', 'configure_command_queue',
            'configure_ai_opponent', 'setup_ai_strategies',
            'get_unit', 'list_units', 'get_building', 'list_buildings'
          ],
          description: 'Strategy action'
        },
        // Unit
        unitId: { type: 'string', description: 'Unit type identifier.' },
        unitName: commonSchemas.stringProp,
        unitCategory: { type: 'string', enum: ['infantry', 'vehicle', 'air', 'naval', 'siege', 'hero', 'worker'], description: 'Unit category.' },
        health: commonSchemas.numberProp,
        armor: commonSchemas.numberProp,
        damage: commonSchemas.numberProp,
        attackSpeed: commonSchemas.numberProp,
        range: commonSchemas.numberProp,
        movementSpeed: commonSchemas.numberProp,
        buildTime: commonSchemas.numberProp,
        cost: commonSchemas.objectProp,
        populationCost: commonSchemas.integerProp,
        abilities: { type: 'array', items: commonSchemas.objectProp, description: 'Unit abilities.' },
        // Building
        buildingId: { type: 'string', description: 'Building identifier.' },
        buildingName: commonSchemas.stringProp,
        buildingCategory: { type: 'string', enum: ['base', 'production', 'tech', 'defense', 'resource', 'support'], description: 'Building category.' },
        trainableUnits: { type: 'array', items: commonSchemas.stringProp, description: 'Units this building can produce.' },
        researchable: { type: 'array', items: commonSchemas.stringProp, description: 'Research available here.' },
        // Resources
        resourceId: { type: 'string', description: 'Resource type identifier.' },
        resourceName: commonSchemas.stringProp,
        gatherRate: commonSchemas.numberProp,
        storageCapacity: commonSchemas.numberProp,
        refinementRequired: commonSchemas.booleanProp,
        // Tech tree
        researchId: { type: 'string', description: 'Research identifier.' },
        researchName: commonSchemas.stringProp,
        researchCost: commonSchemas.objectProp,
        researchTime: commonSchemas.numberProp,
        prerequisites: { type: 'array', items: commonSchemas.stringProp, description: 'Required research.' },
        effects: commonSchemas.objectProp,
        // Fog of war
        fogEnabled: commonSchemas.booleanProp,
        explorationPersistence: commonSchemas.booleanProp,
        sightRange: commonSchemas.numberProp,
        // Formation
        formationId: { type: 'string', description: 'Formation identifier.' },
        formationType: { type: 'string', enum: ['line', 'column', 'box', 'wedge', 'scatter', 'custom'], description: 'Formation type.' },
        formationSpacing: commonSchemas.numberProp,
        stance: { type: 'string', enum: ['aggressive', 'defensive', 'hold_position', 'patrol'], description: 'Unit stance.' },
        // AI
        aiDifficulty: { type: 'string', enum: ['easy', 'normal', 'hard', 'brutal'], description: 'AI difficulty.' },
        aiStrategy: { type: 'string', enum: ['rush', 'turtle', 'balanced', 'economic', 'harassment'], description: 'AI strategy.' },
        aiCheating: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        unitId: commonSchemas.stringProp,
        unitPath: commonSchemas.assetPath,
        buildingId: commonSchemas.stringProp,
        buildingPath: commonSchemas.assetPath,
        researchId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_genre_sports',
    category: 'genre',
    description: 'Sports game mechanics. Create player stats, configure ball physics, setup team AI, create season modes, referee systems, commentary, and replay systems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_player_stats', 'configure_player_attributes', 'configure_stamina',
            'configure_ball_physics', 'setup_ball_behavior',
            'setup_team_ai', 'configure_formations', 'configure_plays',
            'create_season_mode', 'configure_schedule', 'configure_standings', 'configure_playoffs',
            'configure_referee_system', 'add_rule', 'configure_fouls', 'configure_penalties',
            'setup_commentary', 'add_commentary_trigger', 'configure_commentary_bank',
            'create_replay_system', 'configure_replay_cameras',
            'get_team', 'list_teams', 'get_player', 'list_players'
          ],
          description: 'Sports action'
        },
        // Player stats
        playerId: { type: 'string', description: 'Player identifier.' },
        playerName: commonSchemas.stringProp,
        position: commonSchemas.stringProp,
        speed: commonSchemas.numberProp,
        acceleration: commonSchemas.numberProp,
        shooting: commonSchemas.numberProp,
        passing: commonSchemas.numberProp,
        defense: commonSchemas.numberProp,
        stamina: commonSchemas.numberProp,
        staminaRecovery: commonSchemas.numberProp,
        // Ball physics
        ballBounce: commonSchemas.numberProp,
        ballSpin: commonSchemas.booleanProp,
        ballTrajectory: { type: 'string', enum: ['realistic', 'arcade', 'assisted'], description: 'Trajectory calculation.' },
        // Team AI
        teamId: { type: 'string', description: 'Team identifier.' },
        teamName: commonSchemas.stringProp,
        formation: commonSchemas.stringProp,
        playbook: { type: 'array', items: commonSchemas.objectProp, description: 'Available plays.' },
        aiAggression: commonSchemas.numberProp,
        aiPositioning: { type: 'string', enum: ['tight', 'spread', 'adaptive'], description: 'Positioning style.' },
        // Season
        seasonLength: commonSchemas.integerProp,
        schedule: { type: 'array', items: commonSchemas.objectProp, description: 'Match schedule.' },
        playoffFormat: { type: 'string', enum: ['single_elimination', 'double_elimination', 'round_robin', 'bracket'], description: 'Playoff format.' },
        // Referee
        ruleId: { type: 'string', description: 'Rule identifier.' },
        ruleName: commonSchemas.stringProp,
        foulTypes: { type: 'array', items: commonSchemas.stringProp, description: 'Types of fouls.' },
        penaltyTypes: { type: 'array', items: commonSchemas.stringProp, description: 'Penalty options.' },
        cardSystem: commonSchemas.booleanProp,
        // Commentary
        commentaryId: { type: 'string', description: 'Commentary line identifier.' },
        triggerEvent: commonSchemas.stringProp,
        commentaryAudio: commonSchemas.soundPath,
        // Replay
        replayCameras: { type: 'array', items: commonSchemas.objectProp, description: 'Replay camera angles.' },
        slowMotionSpeed: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        playerId: commonSchemas.stringProp,
        teamId: commonSchemas.stringProp,
        seasonId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_genre_horror',
    category: 'genre',
    description: 'Horror game mechanics. Create fear systems, configure atmosphere, setup monster AI, resource scarcity, save systems, jumpscares, and safe zones.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_fear_system', 'configure_sanity', 'configure_fear_effects',
            'configure_atmosphere', 'setup_lighting', 'configure_fog', 'setup_ambient_sounds',
            'setup_monster_ai', 'configure_stalking', 'configure_pursuit', 'configure_retreat',
            'create_resource_scarcity', 'configure_limited_ammo', 'configure_limited_healing',
            'configure_save_system', 'setup_save_rooms', 'configure_limited_saves',
            'setup_jumpscare_system', 'add_jumpscare', 'configure_randomization',
            'create_safe_zones', 'configure_sanctuary_mechanics',
            'get_horror_config', 'test_scare_sequence'
          ],
          description: 'Horror action'
        },
        // Fear system
        sanityMax: commonSchemas.numberProp,
        sanityDecayRate: commonSchemas.numberProp,
        sanityRecoveryRate: commonSchemas.numberProp,
        fearEffects: { type: 'array', items: { type: 'string', enum: ['visual_distortion', 'hallucinations', 'heartbeat', 'breathing', 'screen_shake', 'audio_distortion'] }, description: 'Fear effects.' },
        fearThresholds: commonSchemas.objectProp,
        // Atmosphere
        ambientLightLevel: commonSchemas.numberProp,
        fogDensity: commonSchemas.numberProp,
        fogColor: commonSchemas.objectProp,
        ambientSoundscape: commonSchemas.soundPath,
        musicIntensity: commonSchemas.numberProp,
        // Monster AI
        monsterId: { type: 'string', description: 'Monster identifier.' },
        monsterName: commonSchemas.stringProp,
        stalkingBehavior: { type: 'string', enum: ['passive', 'active', 'triggered', 'persistent'], description: 'Stalking behavior.' },
        pursuitSpeed: commonSchemas.numberProp,
        retreatConditions: commonSchemas.objectProp,
        teleportEnabled: commonSchemas.booleanProp,
        // Resource scarcity
        ammoScarcity: { type: 'string', enum: ['none', 'limited', 'very_limited', 'survival'], description: 'Ammo availability.' },
        healingScarcity: { type: 'string', enum: ['plentiful', 'moderate', 'scarce', 'none'], description: 'Healing availability.' },
        // Save system
        saveType: { type: 'string', enum: ['anywhere', 'save_rooms', 'checkpoint', 'limited_item'], description: 'Save system type.' },
        saveItemName: commonSchemas.stringProp,
        saveRoomLocations: { type: 'array', items: commonSchemas.location, description: 'Save room positions.' },
        // Jumpscares
        jumpscareId: { type: 'string', description: 'Jumpscare identifier.' },
        jumpscareTrigger: { type: 'string', enum: ['proximity', 'interaction', 'timed', 'random', 'scripted'], description: 'Trigger type.' },
        jumpscareVisual: commonSchemas.objectProp,
        jumpscareAudio: commonSchemas.soundPath,
        jumpscareChance: commonSchemas.numberProp,
        // Safe zones
        safeZoneId: { type: 'string', description: 'Safe zone identifier.' },
        safeZoneBounds: commonSchemas.bounds,
        musicChange: commonSchemas.soundPath,
        monstersCanEnter: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        fearSystemId: commonSchemas.stringProp,
        monsterId: commonSchemas.stringProp,
        jumpscareId: commonSchemas.stringProp,
        safeZoneId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_genre_survival',
    category: 'genre',
    description: 'Survival game mechanics. Create needs systems, configure harvesting, setup building, crafting tiers, decay systems, wildlife, and weather effects.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_needs_system', 'configure_hunger', 'configure_thirst', 'configure_sleep', 'configure_temperature',
            'configure_harvesting', 'add_harvestable', 'configure_harvest_tools',
            'setup_building_system', 'add_building_piece', 'configure_snapping', 'configure_stability',
            'create_crafting_tiers', 'add_crafting_station', 'configure_station_recipes',
            'configure_decay_system', 'configure_repair_system',
            'setup_wildlife', 'add_animal', 'configure_taming',
            'create_weather_effects', 'configure_weather_damage',
            'get_survival_config', 'test_needs_balance'
          ],
          description: 'Survival action'
        },
        // Needs
        needId: { type: 'string', description: 'Need identifier.' },
        needName: commonSchemas.stringProp,
        maxValue: commonSchemas.numberProp,
        decayRate: commonSchemas.numberProp,
        criticalThreshold: commonSchemas.numberProp,
        effects: commonSchemas.objectProp,
        // Hunger specific
        hungerDecay: commonSchemas.numberProp,
        starvationDamage: commonSchemas.numberProp,
        // Thirst specific
        thirstDecay: commonSchemas.numberProp,
        dehydrationDamage: commonSchemas.numberProp,
        // Temperature
        comfortZone: { type: 'array', items: commonSchemas.numberProp, description: 'Min/max comfortable temp.' },
        heatDamage: commonSchemas.numberProp,
        coldDamage: commonSchemas.numberProp,
        // Harvesting
        harvestableId: { type: 'string', description: 'Harvestable resource identifier.' },
        harvestableType: { type: 'string', enum: ['tree', 'rock', 'bush', 'ore', 'animal', 'fish'], description: 'Resource type.' },
        requiredTool: commonSchemas.stringProp,
        yieldItems: { type: 'array', items: commonSchemas.objectProp, description: 'Items gained from harvesting.' },
        respawnTime: commonSchemas.numberProp,
        // Building
        buildingPieceId: { type: 'string', description: 'Building piece identifier.' },
        pieceType: { type: 'string', enum: ['foundation', 'wall', 'floor', 'roof', 'door', 'window', 'stairs', 'fence', 'pillar'], description: 'Piece type.' },
        buildingMaterial: { type: 'string', enum: ['wood', 'stone', 'metal', 'thatch', 'concrete'], description: 'Material tier.' },
        health: commonSchemas.numberProp,
        snapPoints: { type: 'array', items: commonSchemas.objectProp, description: 'Snapping configuration.' },
        stabilityRequired: commonSchemas.booleanProp,
        // Crafting
        stationId: { type: 'string', description: 'Crafting station identifier.' },
        stationType: { type: 'string', enum: ['workbench', 'forge', 'chemistry', 'cooking', 'loom', 'tannery'], description: 'Station type.' },
        stationTier: commonSchemas.integerProp,
        fuelRequired: commonSchemas.booleanProp,
        // Decay
        decayEnabled: commonSchemas.booleanProp,
        buildingDecayRate: commonSchemas.numberProp,
        repairCost: commonSchemas.objectProp,
        // Wildlife
        animalId: { type: 'string', description: 'Animal identifier.' },
        animalBehavior: { type: 'string', enum: ['passive', 'neutral', 'aggressive', 'predator', 'prey'], description: 'Animal behavior.' },
        tameable: commonSchemas.booleanProp,
        tamingFood: { type: 'array', items: commonSchemas.stringProp, description: 'Foods for taming.' },
        tamingDifficulty: commonSchemas.numberProp,
        // Weather
        weatherType: { type: 'string', enum: ['rain', 'snow', 'heat_wave', 'storm', 'blizzard'], description: 'Weather type.' },
        weatherDamage: commonSchemas.numberProp,
        weatherDuration: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        needId: commonSchemas.stringProp,
        harvestableId: commonSchemas.stringProp,
        buildingPieceId: commonSchemas.stringProp,
        stationId: commonSchemas.stringProp,
        animalId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_genre_moba',
    category: 'genre',
    description: 'MOBA game mechanics. Create heroes with abilities, configure lane systems, setup minion waves, towers, jungle camps, item shops, and team objectives.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_hero', 'configure_hero_abilities', 'configure_hero_scaling',
            'configure_lane_system', 'setup_lane_paths', 'configure_lane_equilibrium',
            'setup_minion_waves', 'configure_wave_timing', 'configure_minion_types',
            'create_tower_system', 'configure_tower_targeting', 'configure_tower_damage',
            'configure_jungle_camps', 'add_camp', 'configure_buffs', 'configure_respawn',
            'setup_item_shop', 'add_item', 'configure_recipes', 'configure_actives',
            'create_team_objectives', 'configure_objective_rewards',
            'get_hero', 'list_heroes', 'get_item', 'list_items'
          ],
          description: 'MOBA action'
        },
        // Hero
        heroId: { type: 'string', description: 'Hero identifier.' },
        heroName: commonSchemas.stringProp,
        heroRole: { type: 'string', enum: ['carry', 'support', 'tank', 'assassin', 'mage', 'fighter', 'jungler'], description: 'Hero role.' },
        baseHealth: commonSchemas.numberProp,
        baseMana: commonSchemas.numberProp,
        baseAttackDamage: commonSchemas.numberProp,
        baseArmor: commonSchemas.numberProp,
        baseMovementSpeed: commonSchemas.numberProp,
        attackRange: commonSchemas.numberProp,
        abilities: { type: 'array', items: commonSchemas.objectProp, description: 'Hero abilities (Q, W, E, R, passive).' },
        scalingPerLevel: commonSchemas.objectProp,
        // Lanes
        laneType: { type: 'string', enum: ['top', 'mid', 'bot', 'jungle'], description: 'Lane type.' },
        lanePath: { type: 'array', items: commonSchemas.location, description: 'Lane waypoints.' },
        // Minions
        minionType: { type: 'string', enum: ['melee', 'ranged', 'siege', 'super'], description: 'Minion type.' },
        waveInterval: commonSchemas.numberProp,
        wavesComposition: commonSchemas.objectProp,
        goldReward: commonSchemas.numberProp,
        xpReward: commonSchemas.numberProp,
        // Towers
        towerId: { type: 'string', description: 'Tower identifier.' },
        towerTier: commonSchemas.integerProp,
        towerDamage: commonSchemas.numberProp,
        towerHealth: commonSchemas.numberProp,
        towerTargetingPriority: { type: 'array', items: commonSchemas.stringProp, description: 'Target priority order.' },
        towerProtection: commonSchemas.booleanProp,
        // Jungle
        campId: { type: 'string', description: 'Jungle camp identifier.' },
        campMonsters: { type: 'array', items: commonSchemas.objectProp, description: 'Monsters in camp.' },
        campBuff: commonSchemas.objectProp,
        campRespawnTime: commonSchemas.numberProp,
        // Items
        itemId: { type: 'string', description: 'Item identifier.' },
        itemName: commonSchemas.stringProp,
        itemCost: commonSchemas.integerProp,
        itemStats: commonSchemas.objectProp,
        itemPassive: commonSchemas.objectProp,
        itemActive: commonSchemas.objectProp,
        itemRecipe: { type: 'array', items: commonSchemas.stringProp, description: 'Component items.' },
        // Objectives
        objectiveId: { type: 'string', description: 'Objective identifier.' },
        objectiveName: commonSchemas.stringProp,
        objectiveType: { type: 'string', enum: ['baron', 'dragon', 'herald', 'roshan', 'inhibitor', 'nexus'], description: 'Objective type.' },
        objectiveReward: commonSchemas.objectProp,
        objectiveRespawn: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        heroId: commonSchemas.stringProp,
        heroPath: commonSchemas.assetPath,
        itemId: commonSchemas.stringProp,
        itemPath: commonSchemas.assetPath,
        objectiveId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_genre_battle_royale',
    category: 'genre',
    description: 'Battle Royale game mechanics. Create shrinking zones, configure loot spawns, setup drop systems, ping systems, knockdown mechanics, vehicle spawns, and spectator modes.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_shrinking_zone', 'configure_zone_phases', 'configure_zone_damage',
            'configure_loot_spawns', 'add_loot_location', 'configure_rarity_distribution', 'setup_hot_zones',
            'setup_drop_system', 'configure_drop_path', 'configure_drop_timing',
            'create_ping_system', 'add_ping_type', 'configure_ping_visuals',
            'configure_knockdown', 'setup_revive_system', 'configure_finish_mechanics',
            'setup_vehicle_spawns', 'add_vehicle_location', 'configure_vehicle_types',
            'create_spectator_mode', 'configure_spectator_cameras',
            'get_match_config', 'test_zone_timing'
          ],
          description: 'Battle Royale action'
        },
        // Zone
        zoneId: { type: 'string', description: 'Zone phase identifier.' },
        zonePhases: { type: 'array', items: commonSchemas.objectProp, description: 'Zone shrink phases.' },
        zoneDamage: commonSchemas.numberProp,
        zoneDamageScaling: commonSchemas.numberProp,
        zoneVisualType: { type: 'string', enum: ['circle', 'gas', 'storm', 'fire'], description: 'Zone visual style.' },
        finalZoneSize: commonSchemas.numberProp,
        // Loot
        lootLocationId: { type: 'string', description: 'Loot location identifier.' },
        lootLocationType: { type: 'string', enum: ['floor', 'container', 'supply_drop', 'special'], description: 'Loot location type.' },
        lootTier: { type: 'string', enum: ['common', 'uncommon', 'rare', 'legendary', 'mythic'], description: 'Loot tier.' },
        lootDensity: commonSchemas.numberProp,
        hotZoneMultiplier: commonSchemas.numberProp,
        // Drop
        dropType: { type: 'string', enum: ['plane', 'helicopter', 'pod', 'teleport'], description: 'Drop mechanism.' },
        dropPath: { type: 'array', items: commonSchemas.location, description: 'Drop vehicle path.' },
        dropSpeed: commonSchemas.numberProp,
        jumpWindow: commonSchemas.objectProp,
        // Ping
        pingType: { type: 'string', enum: ['enemy', 'loot', 'location', 'danger', 'defend', 'attack'], description: 'Ping type.' },
        pingDuration: commonSchemas.numberProp,
        pingCooldown: commonSchemas.numberProp,
        pingVisual: commonSchemas.objectProp,
        // Knockdown
        knockdownEnabled: commonSchemas.booleanProp,
        knockdownHealth: commonSchemas.numberProp,
        knockdownBleedout: commonSchemas.numberProp,
        reviveTime: commonSchemas.numberProp,
        selfRevive: commonSchemas.booleanProp,
        finishTime: commonSchemas.numberProp,
        // Vehicles
        vehicleSpawnId: { type: 'string', description: 'Vehicle spawn identifier.' },
        vehicleTypes: { type: 'array', items: commonSchemas.stringProp, description: 'Available vehicle types.' },
        vehicleFuel: commonSchemas.booleanProp,
        vehicleDamage: commonSchemas.booleanProp,
        // Spectator
        spectatorModes: { type: 'array', items: { type: 'string', enum: ['follow', 'free_cam', 'killer_cam', 'map_view'] }, description: 'Available spectator modes.' },
        spectatorDelay: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        zoneId: commonSchemas.stringProp,
        lootLocationId: commonSchemas.stringProp,
        vehicleSpawnId: commonSchemas.stringProp,
        matchConfigId: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_genre_rhythm',
    category: 'genre',
    description: 'Rhythm game mechanics. Create note tracks, configure scoring, setup combo systems, create beat map editors, configure difficulty, and visual feedback.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_note_track', 'add_note', 'configure_note_types', 'configure_hold_notes',
            'configure_scoring', 'setup_timing_windows', 'configure_score_multipliers',
            'setup_combo_system', 'configure_combo_breaks', 'configure_streak_bonuses',
            'create_beat_map_editor', 'import_audio', 'configure_bpm_detection',
            'configure_difficulty', 'setup_difficulty_modifiers',
            'setup_visual_feedback', 'configure_hit_effects', 'configure_lane_highlighting',
            'get_song', 'list_songs', 'test_beat_map'
          ],
          description: 'Rhythm action'
        },
        // Note track
        trackId: { type: 'string', description: 'Track identifier.' },
        trackName: commonSchemas.stringProp,
        audioPath: commonSchemas.soundPath,
        bpm: commonSchemas.numberProp,
        offset: commonSchemas.numberProp,
        laneCount: commonSchemas.integerProp,
        // Notes
        noteId: { type: 'string', description: 'Note identifier.' },
        noteType: { type: 'string', enum: ['tap', 'hold', 'slide', 'flick', 'drag'], description: 'Note type.' },
        lane: commonSchemas.integerProp,
        timing: commonSchemas.numberProp,
        holdDuration: commonSchemas.numberProp,
        slideDirection: { type: 'string', enum: ['left', 'right', 'up', 'down'], description: 'Slide direction.' },
        // Scoring
        scoringTier: { type: 'string', enum: ['perfect', 'great', 'good', 'ok', 'miss'], description: 'Scoring tier.' },
        timingWindow: commonSchemas.numberProp,
        scoreValue: commonSchemas.integerProp,
        // Combo
        comboMultiplier: { type: 'array', items: commonSchemas.numberProp, description: 'Multiplier at combo thresholds.' },
        comboThresholds: { type: 'array', items: commonSchemas.integerProp, description: 'Combo thresholds for multipliers.' },
        streakBonus: commonSchemas.integerProp,
        // Beat map
        beatMapId: { type: 'string', description: 'Beat map identifier.' },
        autoBPM: commonSchemas.booleanProp,
        snapToGrid: commonSchemas.booleanProp,
        gridDivision: commonSchemas.integerProp,
        // Difficulty
        difficulty: { type: 'string', enum: ['easy', 'normal', 'hard', 'expert', 'master'], description: 'Difficulty level.' },
        noteSpeed: commonSchemas.numberProp,
        noteDensity: commonSchemas.numberProp,
        // Visual feedback
        hitEffect: commonSchemas.niagaraPath,
        missEffect: commonSchemas.niagaraPath,
        laneHighlightColor: commonSchemas.objectProp,
        backgroundVideo: commonSchemas.stringProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        trackId: commonSchemas.stringProp,
        trackPath: commonSchemas.assetPath,
        beatMapId: commonSchemas.stringProp,
        beatMapPath: commonSchemas.assetPath
      }
    }
  },

  {
    name: 'manage_genre_card',
    category: 'genre',
    description: 'Card/deck-building game mechanics. Create cards, configure deck rules, setup turn systems, manage mana systems, configure card effects, and deck builders.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_card', 'configure_card_stats', 'add_card_effect', 'configure_card_art',
            'configure_deck_rules', 'set_deck_size', 'configure_copies_limit', 'setup_banned_list',
            'setup_turn_system', 'configure_phases', 'configure_turn_timer',
            'create_mana_system', 'configure_mana_generation', 'configure_mana_spending',
            'configure_card_effects', 'add_effect_type', 'configure_targeting',
            'setup_deck_builder', 'configure_collection', 'configure_filtering', 'configure_saving',
            'get_card', 'list_cards', 'get_deck', 'list_decks', 'validate_deck'
          ],
          description: 'Card game action'
        },
        // Card
        cardId: { type: 'string', description: 'Card identifier.' },
        cardName: commonSchemas.stringProp,
        cardType: { type: 'string', enum: ['creature', 'spell', 'artifact', 'enchantment', 'land', 'instant', 'trap'], description: 'Card type.' },
        cardRarity: { type: 'string', enum: ['common', 'uncommon', 'rare', 'epic', 'legendary'], description: 'Card rarity.' },
        manaCost: commonSchemas.integerProp,
        manaColor: { type: 'array', items: { type: 'string', enum: ['red', 'blue', 'green', 'white', 'black', 'colorless'] }, description: 'Mana colors required.' },
        attack: commonSchemas.integerProp,
        health: commonSchemas.integerProp,
        cardText: commonSchemas.stringProp,
        flavorText: commonSchemas.stringProp,
        artPath: commonSchemas.texturePath,
        // Deck rules
        minDeckSize: commonSchemas.integerProp,
        maxDeckSize: commonSchemas.integerProp,
        maxCopies: commonSchemas.integerProp,
        bannedCards: { type: 'array', items: commonSchemas.stringProp, description: 'Banned card IDs.' },
        restrictedCards: { type: 'array', items: commonSchemas.stringProp, description: 'Restricted card IDs (1 copy max).' },
        // Turn system
        phases: { type: 'array', items: { type: 'string', enum: ['draw', 'main', 'combat', 'end', 'standby', 'battle'] }, description: 'Turn phases.' },
        drawCount: commonSchemas.integerProp,
        handLimit: commonSchemas.integerProp,
        turnTimer: commonSchemas.numberProp,
        // Mana
        manaType: { type: 'string', enum: ['regenerating', 'accumulating', 'land_based', 'fixed'], description: 'Mana system type.' },
        startingMana: commonSchemas.integerProp,
        maxMana: commonSchemas.integerProp,
        manaPerTurn: commonSchemas.integerProp,
        // Effects
        effectType: { type: 'string', enum: ['damage', 'heal', 'draw', 'discard', 'buff', 'debuff', 'summon', 'destroy', 'transform', 'copy'], description: 'Effect type.' },
        effectValue: commonSchemas.integerProp,
        targetType: { type: 'string', enum: ['self', 'enemy', 'ally', 'any', 'all_enemies', 'all_allies', 'all'], description: 'Targeting type.' },
        targetCount: commonSchemas.integerProp,
        // Deck builder
        collectionId: { type: 'string', description: 'Collection identifier.' },
        filterCategories: { type: 'array', items: commonSchemas.stringProp, description: 'Available filter options.' },
        sortOptions: { type: 'array', items: commonSchemas.stringProp, description: 'Available sort options.' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        cardId: commonSchemas.stringProp,
        cardPath: commonSchemas.assetPath,
        deckId: commonSchemas.stringProp,
        deckPath: commonSchemas.assetPath,
        isValid: commonSchemas.booleanProp,
        validationErrors: commonSchemas.arrayOfStrings
      }
    }
  },
  // ============================================================================
  // Phase 3: Camera System
  // ============================================================================
  {
    name: 'manage_camera',
    category: 'gameplay',
    description: 'Configure camera modes, collision, shake, transitions, and split-screen for various game types.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_camera_mode', 'configure_camera_collision', 'setup_camera_shake', 'create_camera_target',
            'configure_camera_lag', 'setup_camera_constraints', 'create_camera_transition', 'configure_split_screen',
            'get_camera_info', 'list_camera_modes'
          ],
          description: 'Camera action to perform'
        },
        modeName: commonSchemas.stringProp,
        cameraType: { type: 'string', enum: ['third_person', 'first_person', 'isometric', 'side_scroller', 'top_down', 'orbit'], description: 'Camera mode type.' },
        armLength: commonSchemas.numberProp,
        fov: commonSchemas.numberProp,
        collisionMode: { type: 'string', enum: ['push', 'cut', 'fade', 'none'], description: 'Camera collision handling.' },
        collisionRadius: commonSchemas.numberProp,
        shakeType: { type: 'string', enum: ['perlin', 'oscillation', 'impact', 'custom'], description: 'Camera shake type.' },
        shakeDuration: commonSchemas.numberProp,
        shakeAmplitude: commonSchemas.numberProp,
        shakeFrequency: commonSchemas.numberProp,
        positionLag: commonSchemas.numberProp,
        rotationLag: commonSchemas.numberProp,
        pitchMin: commonSchemas.numberProp,
        pitchMax: commonSchemas.numberProp,
        distanceMin: commonSchemas.numberProp,
        distanceMax: commonSchemas.numberProp,
        transitionType: { type: 'string', enum: ['blend', 'cut', 'ease_in', 'ease_out'], description: 'Camera transition type.' },
        transitionDuration: commonSchemas.numberProp,
        splitScreenMode: { type: 'string', enum: ['horizontal', 'vertical', 'dynamic', 'quadrant'], description: 'Split screen mode.' },
        playerCount: commonSchemas.integerProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        modeId: commonSchemas.stringProp,
        modePath: commonSchemas.assetPath
      }
    }
  },
  // ============================================================================
  // Phase 3: Physics System
  // ============================================================================
  {
    name: 'manage_physics',
    category: 'gameplay',
    description: 'Configure physics materials, constraints, volumes, ragdolls, and projectile physics.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_physics_material', 'configure_physics_constraints', 'setup_physics_volumes',
            'create_ragdoll', 'configure_ragdoll', 'setup_projectile_physics',
            'get_physics_info', 'list_physics_materials'
          ],
          description: 'Physics action to perform'
        },
        materialName: commonSchemas.stringProp,
        friction: commonSchemas.numberProp,
        restitution: commonSchemas.numberProp,
        density: commonSchemas.numberProp,
        constraintType: { type: 'string', enum: ['hinge', 'ball_socket', 'prismatic', 'fixed', 'free'], description: 'Constraint type.' },
        linearLimits: commonSchemas.objectProp,
        angularLimits: commonSchemas.objectProp,
        volumeType: { type: 'string', enum: ['water', 'low_gravity', 'wind', 'kill', 'pain'], description: 'Physics volume type.' },
        buoyancy: commonSchemas.numberProp,
        gravityScale: commonSchemas.numberProp,
        windStrength: commonSchemas.numberProp,
        skeletalMesh: commonSchemas.meshPath,
        ragdollProfile: commonSchemas.stringProp,
        blendWeight: commonSchemas.numberProp,
        projectileSpeed: commonSchemas.numberProp,
        gravityEffect: commonSchemas.numberProp,
        penetrationDepth: commonSchemas.numberProp,
        ricochetAngle: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        materialPath: commonSchemas.assetPath,
        constraintId: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 3: Damage System
  // ============================================================================
  {
    name: 'manage_damage',
    category: 'gameplay',
    description: 'Configure damage types, calculations, critical hits, damage numbers, and death handling.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_damage_type', 'configure_damage_falloff', 'setup_critical_hits',
            'create_damage_numbers', 'configure_death_handling', 'setup_invincibility_frames',
            'get_damage_type_info', 'list_damage_types'
          ],
          description: 'Damage action to perform'
        },
        damageTypeName: commonSchemas.stringProp,
        damageCategory: { type: 'string', enum: ['physical', 'magical', 'fire', 'ice', 'lightning', 'poison', 'holy', 'dark', 'true'], description: 'Damage category.' },
        baseDamage: commonSchemas.numberProp,
        falloffType: { type: 'string', enum: ['none', 'linear', 'exponential', 'curve'], description: 'Damage falloff type.' },
        falloffStart: commonSchemas.numberProp,
        falloffEnd: commonSchemas.numberProp,
        criticalChance: commonSchemas.numberProp,
        criticalMultiplier: commonSchemas.numberProp,
        headshotMultiplier: commonSchemas.numberProp,
        showDamageNumbers: commonSchemas.booleanProp,
        damageNumberColors: commonSchemas.objectProp,
        floatSpeed: commonSchemas.numberProp,
        deathType: { type: 'string', enum: ['respawn', 'permadeath', 'checkpoint', 'game_over'], description: 'Death handling type.' },
        respawnDelay: commonSchemas.numberProp,
        iframeDuration: commonSchemas.numberProp,
        iframeTriggers: { type: 'array', items: { type: 'string', enum: ['hit', 'respawn', 'dodge', 'parry'] }, description: 'I-frame triggers.' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        damageTypeId: commonSchemas.stringProp,
        damageTypePath: commonSchemas.assetPath
      }
    }
  },
  // ============================================================================
  // Phase 6: Nanite
  // ============================================================================
  {
    name: 'manage_nanite',
    category: 'core',
    description: 'Configure Nanite virtualized geometry for meshes and foliage.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'enable_nanite', 'disable_nanite', 'configure_nanite_settings',
            'setup_nanite_foliage', 'optimize_nanite_scene', 'get_nanite_stats'
          ],
          description: 'Nanite action to perform'
        },
        meshPath: commonSchemas.meshPath,
        enabled: commonSchemas.booleanProp,
        fallbackPercent: commonSchemas.numberProp,
        preserveArea: commonSchemas.booleanProp,
        positionPrecision: commonSchemas.integerProp,
        targetPixelsPerEdge: commonSchemas.numberProp,
        foliageType: commonSchemas.stringProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        triangleCount: commonSchemas.integerProp,
        clusterCount: commonSchemas.integerProp,
        memoryUsage: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 6: Lumen
  // ============================================================================
  {
    name: 'manage_lumen',
    category: 'core',
    description: 'Configure Lumen global illumination and reflections.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'configure_lumen_gi', 'setup_lumen_reflections', 'configure_emissive_lighting',
            'optimize_lumen_performance', 'get_lumen_stats'
          ],
          description: 'Lumen action to perform'
        },
        quality: { type: 'string', enum: ['low', 'medium', 'high', 'epic'], description: 'Lumen quality preset.' },
        traceDistance: commonSchemas.numberProp,
        bounces: commonSchemas.integerProp,
        useHardwareRT: commonSchemas.booleanProp,
        reflectionMethod: { type: 'string', enum: ['screen_space', 'software_rt', 'hardware_rt'], description: 'Reflection method.' },
        reflectionQuality: commonSchemas.numberProp,
        emissiveBoost: commonSchemas.numberProp,
        rayBudget: commonSchemas.integerProp,
        detailTracing: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        performance: commonSchemas.objectProp
      }
    }
  },
  // ============================================================================
  // Phase 6: MetaSounds
  // ============================================================================
  {
    name: 'manage_metasounds',
    category: 'core',
    description: 'Create and configure MetaSounds for procedural audio.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_metasound', 'add_oscillator', 'add_filter', 'add_envelope',
            'create_procedural_sfx', 'create_dynamic_music', 'configure_spatialization',
            'connect_nodes', 'get_metasound_info', 'list_metasounds'
          ],
          description: 'MetaSounds action to perform'
        },
        metasoundName: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        oscillatorType: { type: 'string', enum: ['sine', 'saw', 'square', 'triangle', 'noise'], description: 'Oscillator waveform.' },
        frequency: commonSchemas.numberProp,
        amplitude: commonSchemas.numberProp,
        filterType: { type: 'string', enum: ['low_pass', 'high_pass', 'band_pass', 'notch'], description: 'Filter type.' },
        cutoffFrequency: commonSchemas.numberProp,
        resonance: commonSchemas.numberProp,
        attack: commonSchemas.numberProp,
        decay: commonSchemas.numberProp,
        sustain: commonSchemas.numberProp,
        release: commonSchemas.numberProp,
        sfxType: { type: 'string', enum: ['footstep', 'impact', 'ambience', 'ui', 'weapon'], description: 'SFX category.' },
        attenuationSettings: commonSchemas.objectProp,
        sourceNodeId: commonSchemas.stringProp,
        targetNodeId: commonSchemas.stringProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        metasoundPath: commonSchemas.assetPath,
        nodeId: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 6: Control Rig
  // ============================================================================
  {
    name: 'manage_control_rig',
    category: 'core',
    description: 'Create and configure Control Rigs for procedural animation.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_control_rig', 'setup_control_rig_physics', 'configure_full_body_ik',
            'create_procedural_animation', 'setup_motion_matching', 'configure_ragdoll_blend',
            'add_control', 'get_control_rig_info'
          ],
          description: 'Control Rig action to perform'
        },
        rigName: commonSchemas.stringProp,
        skeletalMesh: commonSchemas.meshPath,
        savePath: commonSchemas.savePath,
        ikChain: commonSchemas.stringProp,
        ikTarget: commonSchemas.stringProp,
        physicsEnabled: commonSchemas.booleanProp,
        physicsBlend: commonSchemas.numberProp,
        footPlacement: commonSchemas.booleanProp,
        handIK: commonSchemas.booleanProp,
        animationType: { type: 'string', enum: ['breathing', 'look_at', 'lean', 'additive'], description: 'Procedural animation type.' },
        motionMatchingDatabase: commonSchemas.assetPath,
        ragdollBlendTime: commonSchemas.numberProp,
        controlName: commonSchemas.stringProp,
        controlType: { type: 'string', enum: ['transform', 'float', 'bool', 'vector'], description: 'Control type.' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        rigPath: commonSchemas.assetPath,
        controlId: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 6: Chaos Physics & Destruction
  // ============================================================================
  {
    name: 'manage_chaos',
    category: 'core',
    description: 'Configure Chaos physics, destruction, vehicles, and cloth simulation.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_destructible', 'configure_destruction_levels', 'setup_chaos_vehicle',
            'configure_chaos_cloth', 'create_physics_field', 'setup_networked_physics',
            'fracture_mesh', 'get_chaos_stats'
          ],
          description: 'Chaos action to perform'
        },
        meshPath: commonSchemas.meshPath,
        savePath: commonSchemas.savePath,
        fractureMode: { type: 'string', enum: ['voronoi', 'uniform', 'radial', 'clustered'], description: 'Fracture algorithm.' },
        clusterCount: commonSchemas.integerProp,
        damageThreshold: commonSchemas.numberProp,
        impactResistance: commonSchemas.numberProp,
        debrisLifetime: commonSchemas.numberProp,
        vehicleType: { type: 'string', enum: ['wheeled', 'tracked', 'hover', 'boat'], description: 'Vehicle physics type.' },
        wheelCount: commonSchemas.integerProp,
        suspensionSettings: commonSchemas.objectProp,
        clothSettings: commonSchemas.objectProp,
        fieldType: { type: 'string', enum: ['force', 'noise', 'kill', 'sleep'], description: 'Physics field type.' },
        fieldStrength: commonSchemas.numberProp,
        networkPrediction: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        geometryCollectionPath: commonSchemas.assetPath,
        pieceCount: commonSchemas.integerProp
      }
    }
  },
  // ============================================================================
  // Phase 6: Cinematics
  // ============================================================================
  {
    name: 'manage_cinematics',
    category: 'authoring',
    description: 'Create and configure level sequences, cutscenes, and cinematic cameras.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_level_sequence', 'add_camera_track', 'add_actor_track', 'add_audio_track',
            'add_event_track', 'configure_camera_rig', 'setup_cinematic_mode',
            'add_keyframe', 'get_sequence_info', 'list_sequences'
          ],
          description: 'Cinematics action to perform'
        },
        sequenceName: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        duration: commonSchemas.numberProp,
        frameRate: commonSchemas.numberProp,
        cameraActor: commonSchemas.stringProp,
        trackType: { type: 'string', enum: ['transform', 'property', 'event', 'audio', 'fade'], description: 'Track type.' },
        targetActor: commonSchemas.stringProp,
        audioAsset: commonSchemas.assetPath,
        eventName: commonSchemas.eventName,
        cameraRigType: { type: 'string', enum: ['rail', 'crane', 'handheld', 'static', 'drone'], description: 'Camera rig type.' },
        letterboxRatio: commonSchemas.numberProp,
        depthOfField: commonSchemas.booleanProp,
        motionBlur: commonSchemas.booleanProp,
        time: commonSchemas.numberProp,
        value: commonSchemas.value,
        interpolation: { type: 'string', enum: ['linear', 'cubic', 'constant'], description: 'Keyframe interpolation.' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        sequencePath: commonSchemas.assetPath,
        trackId: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 7: Materials
  // ============================================================================
  {
    name: 'manage_materials',
    category: 'authoring',
    description: 'Create and configure PBR materials, material instances, functions, and landscape materials.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_material', 'configure_material_parameters', 'setup_material_instances',
            'create_material_functions', 'configure_material_layers', 'setup_procedural_textures',
            'create_landscape_material', 'configure_decal_material', 'setup_subsurface_scattering',
            'create_cloth_material', 'get_material_info', 'list_materials'
          ],
          description: 'Materials action to perform'
        },
        materialName: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        materialType: { type: 'string', enum: ['pbr', 'unlit', 'subsurface', 'clear_coat', 'cloth', 'two_sided_foliage'], description: 'Material type.' },
        baseColor: commonSchemas.objectProp,
        metallic: commonSchemas.numberProp,
        roughness: commonSchemas.numberProp,
        normalMap: commonSchemas.texturePath,
        emissiveColor: commonSchemas.objectProp,
        opacity: commonSchemas.numberProp,
        blendMode: { type: 'string', enum: ['opaque', 'masked', 'translucent', 'additive', 'modulate'], description: 'Blend mode.' },
        parentMaterial: commonSchemas.materialPath,
        parameters: commonSchemas.objectProp,
        layerCount: commonSchemas.integerProp,
        blendType: { type: 'string', enum: ['height', 'alpha', 'vertex_color'], description: 'Layer blend type.' },
        subsurfaceColor: commonSchemas.objectProp,
        subsurfaceRadius: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        materialPath: commonSchemas.assetPath
      }
    }
  },
  // ============================================================================
  // Phase 7: VFX
  // ============================================================================
  {
    name: 'manage_vfx',
    category: 'authoring',
    description: 'Create and configure Niagara particle systems and effects.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_niagara_system', 'add_niagara_module', 'configure_particle_spawn',
            'setup_particle_attributes', 'create_particle_forces', 'configure_collision_particles',
            'setup_gpu_particles', 'create_ribbon_renderer', 'configure_mesh_renderer',
            'setup_sprite_renderer', 'create_vfx_event', 'configure_vfx_pooling',
            'get_vfx_info', 'list_vfx_systems'
          ],
          description: 'VFX action to perform'
        },
        systemName: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        emitterName: commonSchemas.stringProp,
        moduleType: commonSchemas.stringProp,
        spawnRate: commonSchemas.numberProp,
        burstCount: commonSchemas.integerProp,
        lifetime: commonSchemas.numberProp,
        initialSize: commonSchemas.numberProp,
        initialVelocity: commonSchemas.objectProp,
        initialColor: commonSchemas.objectProp,
        forceType: { type: 'string', enum: ['drag', 'noise', 'curl', 'vortex', 'gravity', 'point_attractor'], description: 'Force type.' },
        forceStrength: commonSchemas.numberProp,
        collisionEnabled: commonSchemas.booleanProp,
        bounceAmount: commonSchemas.numberProp,
        meshToSpawn: commonSchemas.meshPath,
        ribbonWidth: commonSchemas.numberProp,
        eventType: { type: 'string', enum: ['impact', 'muzzle_flash', 'blood_splatter', 'explosion', 'magic'], description: 'VFX event type.' },
        poolSize: commonSchemas.integerProp,
        prewarm: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        systemPath: commonSchemas.assetPath,
        emitterId: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 7: Post Process
  // ============================================================================
  {
    name: 'manage_post_process',
    category: 'authoring',
    description: 'Configure post-processing effects including bloom, color grading, DOF, and screen effects.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_post_process_volume', 'configure_bloom', 'setup_color_grading',
            'create_depth_of_field', 'configure_motion_blur', 'setup_ambient_occlusion',
            'create_screen_effects', 'configure_outline_effect', 'setup_fog',
            'get_post_process_info', 'list_post_process_volumes'
          ],
          description: 'Post process action to perform'
        },
        volumeName: commonSchemas.stringProp,
        priority: commonSchemas.integerProp,
        blendRadius: commonSchemas.numberProp,
        isUnbound: commonSchemas.booleanProp,
        bloomIntensity: commonSchemas.numberProp,
        bloomThreshold: commonSchemas.numberProp,
        bloomTint: commonSchemas.objectProp,
        lutTexture: commonSchemas.texturePath,
        temperature: commonSchemas.numberProp,
        tint: commonSchemas.objectProp,
        saturation: commonSchemas.numberProp,
        contrast: commonSchemas.numberProp,
        dofMethod: { type: 'string', enum: ['gaussian', 'bokeh', 'cinematic'], description: 'DOF method.' },
        focalDistance: commonSchemas.numberProp,
        focalRegion: commonSchemas.numberProp,
        aperture: commonSchemas.numberProp,
        motionBlurAmount: commonSchemas.numberProp,
        motionBlurMax: commonSchemas.numberProp,
        aoIntensity: commonSchemas.numberProp,
        aoRadius: commonSchemas.numberProp,
        vignetteIntensity: commonSchemas.numberProp,
        grainIntensity: commonSchemas.numberProp,
        chromaticAberration: commonSchemas.numberProp,
        fogDensity: commonSchemas.numberProp,
        fogColor: commonSchemas.objectProp,
        fogHeight: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        volumePath: commonSchemas.assetPath
      }
    }
  },
  // ============================================================================
  // Phase 8: Music
  // ============================================================================
  {
    name: 'manage_music',
    category: 'authoring',
    description: 'Create and configure dynamic music systems with layers, states, and adaptive transitions.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_music_layer', 'configure_music_state', 'setup_stinger_system',
            'create_adaptive_music', 'configure_music_transitions', 'setup_rhythm_sync',
            'get_music_info', 'list_music_tracks'
          ],
          description: 'Music action to perform'
        },
        trackName: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        layerName: commonSchemas.stringProp,
        audioAsset: commonSchemas.assetPath,
        layerVolume: commonSchemas.numberProp,
        fadeInTime: commonSchemas.numberProp,
        fadeOutTime: commonSchemas.numberProp,
        musicState: { type: 'string', enum: ['explore', 'combat', 'stealth', 'boss', 'victory', 'defeat', 'menu'], description: 'Music state.' },
        stingerType: { type: 'string', enum: ['hit', 'death', 'pickup', 'level_up', 'discovery'], description: 'Stinger event type.' },
        intensityParameter: commonSchemas.stringProp,
        dangerParameter: commonSchemas.stringProp,
        transitionType: { type: 'string', enum: ['crossfade', 'sync_point', 'immediate', 'on_beat'], description: 'Transition type.' },
        bpm: commonSchemas.numberProp,
        beatsPerBar: commonSchemas.integerProp,
        quantization: { type: 'string', enum: ['beat', 'bar', 'half_bar', 'none'], description: 'Rhythm quantization.' },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        trackPath: commonSchemas.assetPath,
        layerId: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 10: MMO Infrastructure
  // ============================================================================
  {
    name: 'manage_mmo',
    category: 'gameplay',
    description: 'Configure MMO-specific systems: world servers, sharding, instances, guilds, and raids.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'setup_world_server', 'configure_sharding', 'create_instance',
            'setup_login_server', 'configure_chat_system', 'setup_mail_system',
            'create_guild_system', 'configure_party_system', 'setup_raid_system',
            'get_mmo_status', 'list_instances'
          ],
          description: 'MMO action to perform'
        },
        serverName: commonSchemas.stringProp,
        maxPlayers: commonSchemas.integerProp,
        shardType: { type: 'string', enum: ['zone', 'channel', 'layer', 'megaserver'], description: 'Sharding type.' },
        shardCapacity: commonSchemas.integerProp,
        instanceType: { type: 'string', enum: ['dungeon', 'raid', 'battleground', 'arena', 'housing'], description: 'Instance type.' },
        instanceMap: commonSchemas.assetPath,
        maxPartySize: commonSchemas.integerProp,
        chatChannels: { type: 'array', items: { type: 'string', enum: ['global', 'zone', 'party', 'guild', 'whisper', 'trade'] }, description: 'Chat channels.' },
        mailAttachments: commonSchemas.booleanProp,
        mailCod: commonSchemas.booleanProp,
        guildMaxMembers: commonSchemas.integerProp,
        guildRanks: commonSchemas.integerProp,
        raidSize: commonSchemas.integerProp,
        lockoutDuration: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        serverId: commonSchemas.stringProp,
        instanceId: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 10: Social Features
  // ============================================================================
  {
    name: 'manage_social',
    category: 'gameplay',
    description: 'Configure social features: friends, presence, voice chat, emotes, and player housing.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_friends_system', 'setup_presence', 'configure_voice_chat',
            'create_emote_system', 'setup_player_housing', 'configure_trading',
            'get_social_status', 'list_emotes'
          ],
          description: 'Social action to perform'
        },
        maxFriends: commonSchemas.integerProp,
        blockListEnabled: commonSchemas.booleanProp,
        presenceStates: { type: 'array', items: { type: 'string', enum: ['online', 'away', 'busy', 'invisible', 'in_game'] }, description: 'Presence states.' },
        voiceChatEnabled: commonSchemas.booleanProp,
        voiceChannels: { type: 'array', items: { type: 'string', enum: ['party', 'guild', 'proximity', 'custom'] }, description: 'Voice channels.' },
        emoteName: commonSchemas.stringProp,
        emoteAnimation: commonSchemas.assetPath,
        emoteCategory: { type: 'string', enum: ['greeting', 'dance', 'taunt', 'laugh', 'sit', 'custom'], description: 'Emote category.' },
        housingEnabled: commonSchemas.booleanProp,
        plotSize: { type: 'string', enum: ['small', 'medium', 'large', 'mansion'], description: 'Housing plot size.' },
        visitingEnabled: commonSchemas.booleanProp,
        tradingEnabled: commonSchemas.booleanProp,
        tradeDistance: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        emoteId: commonSchemas.stringProp,
        housingId: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 12: Save System
  // ============================================================================
  {
    name: 'manage_save_system',
    category: 'utility',
    description: 'Configure save/load system with autosave, multiple slots, and cloud saves.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_save_game', 'configure_autosave', 'setup_save_slots',
            'create_cloud_save', 'configure_save_migration',
            'get_save_info', 'list_save_slots'
          ],
          description: 'Save system action to perform'
        },
        saveGameClass: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        autosaveEnabled: commonSchemas.booleanProp,
        autosaveInterval: commonSchemas.numberProp,
        autosaveTriggers: { type: 'array', items: { type: 'string', enum: ['checkpoint', 'level_change', 'quest_complete', 'time_interval'] }, description: 'Autosave triggers.' },
        maxSlots: commonSchemas.integerProp,
        showThumbnails: commonSchemas.booleanProp,
        cloudSyncEnabled: commonSchemas.booleanProp,
        cloudProvider: { type: 'string', enum: ['steam', 'epic', 'platform', 'custom'], description: 'Cloud save provider.' },
        conflictResolution: { type: 'string', enum: ['newest', 'oldest', 'ask_user', 'merge'], description: 'Conflict resolution strategy.' },
        versionMigration: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        saveGamePath: commonSchemas.assetPath,
        slotCount: commonSchemas.integerProp
      }
    }
  },
  // ============================================================================
  // Phase 12: UI System
  // ============================================================================
  {
    name: 'manage_ui_system',
    category: 'authoring',
    description: 'Create HUD, menus, inventory UI, dialogue UI, and notification systems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_hud', 'create_menu', 'create_inventory_ui',
            'create_dialogue_ui', 'setup_notification_system', 'configure_input_hints',
            'create_tutorial_system', 'get_ui_info', 'list_ui_widgets'
          ],
          description: 'UI system action to perform'
        },
        widgetName: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        hudElements: { type: 'array', items: { type: 'string', enum: ['health', 'ammo', 'minimap', 'compass', 'crosshair', 'objectives'] }, description: 'HUD elements.' },
        menuType: { type: 'string', enum: ['main', 'pause', 'settings', 'inventory', 'character', 'map'], description: 'Menu type.' },
        inventoryLayout: { type: 'string', enum: ['grid', 'list', 'equipment_slots', 'hybrid'], description: 'Inventory layout.' },
        gridSize: commonSchemas.objectProp,
        dialogueStyle: { type: 'string', enum: ['subtitle', 'bubble', 'fullscreen', 'side_portrait'], description: 'Dialogue display style.' },
        notificationTypes: { type: 'array', items: { type: 'string', enum: ['toast', 'popup', 'alert', 'achievement'] }, description: 'Notification types.' },
        inputHintStyle: { type: 'string', enum: ['icons', 'text', 'combined'], description: 'Input hint style.' },
        tutorialSteps: commonSchemas.arrayOfObjects,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        widgetPath: commonSchemas.assetPath
      }
    }
  },
  // ============================================================================
  // Phase 12: Accessibility
  // ============================================================================
  {
    name: 'manage_accessibility',
    category: 'utility',
    description: 'Configure accessibility features: colorblind modes, subtitles, audio cues, and input options.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'configure_colorblind_modes', 'setup_subtitle_options', 'create_audio_cues',
            'configure_input_remapping', 'setup_difficulty_options', 'create_screen_reader',
            'configure_motor_accessibility', 'get_accessibility_info'
          ],
          description: 'Accessibility action to perform'
        },
        colorblindMode: { type: 'string', enum: ['none', 'protanopia', 'deuteranopia', 'tritanopia'], description: 'Colorblind mode.' },
        colorblindStrength: commonSchemas.numberProp,
        subtitleSize: { type: 'string', enum: ['small', 'medium', 'large', 'extra_large'], description: 'Subtitle size.' },
        subtitleBackground: commonSchemas.booleanProp,
        speakerNames: commonSchemas.booleanProp,
        closedCaptions: commonSchemas.booleanProp,
        audioCueType: { type: 'string', enum: ['spatial', 'directional', 'alert'], description: 'Audio cue type.' },
        fullRemapping: commonSchemas.booleanProp,
        holdToToggle: commonSchemas.booleanProp,
        autoAim: commonSchemas.booleanProp,
        autoAimStrength: commonSchemas.numberProp,
        difficultyPresets: { type: 'array', items: { type: 'string', enum: ['easy', 'normal', 'hard', 'custom', 'assist'] }, description: 'Difficulty presets.' },
        screenReaderEnabled: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        settingsApplied: commonSchemas.booleanProp
      }
    }
  },
  // ============================================================================
  // Phase 12: Localization
  // ============================================================================
  {
    name: 'manage_localization',
    category: 'utility',
    description: 'Configure localization: string tables, text formatting, audio localization, and RTL support.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'setup_string_tables', 'configure_text_formatting', 'setup_audio_localization',
            'create_font_fallbacks', 'configure_rtl_support', 'setup_regional_content',
            'get_localization_info', 'list_languages'
          ],
          description: 'Localization action to perform'
        },
        tableName: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        sourceLanguage: commonSchemas.stringProp,
        targetLanguages: commonSchemas.arrayOfStrings,
        pluralRules: commonSchemas.objectProp,
        genderRules: commonSchemas.objectProp,
        numberFormat: commonSchemas.objectProp,
        dateFormat: commonSchemas.stringProp,
        dubbedLanguages: commonSchemas.arrayOfStrings,
        subtitledLanguages: commonSchemas.arrayOfStrings,
        fontFallbacks: commonSchemas.objectProp,
        rtlLanguages: commonSchemas.arrayOfStrings,
        mirrorUI: commonSchemas.booleanProp,
        regionalAssets: commonSchemas.objectProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        tablePath: commonSchemas.assetPath,
        languageCount: commonSchemas.integerProp
      }
    }
  },
  // ============================================================================
  // Phase 12: Photo Mode
  // ============================================================================
  {
    name: 'manage_photo_mode',
    category: 'utility',
    description: 'Create and configure photo mode with camera controls, filters, poses, and frame options.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_photo_mode', 'configure_camera_controls', 'setup_filters',
            'create_poses', 'configure_hide_ui', 'setup_depth_of_field',
            'create_frame_options', 'get_photo_mode_info'
          ],
          description: 'Photo mode action to perform'
        },
        photoModeName: commonSchemas.stringProp,
        savePath: commonSchemas.savePath,
        fovRange: commonSchemas.arrayOfNumbers,
        rollRange: commonSchemas.arrayOfNumbers,
        maxDistance: commonSchemas.numberProp,
        filterName: commonSchemas.stringProp,
        filterLut: commonSchemas.texturePath,
        filterIntensity: commonSchemas.numberProp,
        poseName: commonSchemas.stringProp,
        poseAnimation: commonSchemas.assetPath,
        hideCharacter: commonSchemas.booleanProp,
        hideEffects: commonSchemas.booleanProp,
        bokehShape: { type: 'string', enum: ['circle', 'hexagon', 'octagon', 'custom'], description: 'Bokeh shape.' },
        focusMode: { type: 'string', enum: ['manual', 'auto', 'face_detect'], description: 'Focus mode.' },
        borderStyle: { type: 'string', enum: ['none', 'letterbox', 'vignette', 'frame'], description: 'Border style.' },
        watermark: commonSchemas.booleanProp,
        timestamp: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        photoModePath: commonSchemas.assetPath,
        filterId: commonSchemas.stringProp,
        poseId: commonSchemas.stringProp
      }
    }
  },
  // ============================================================================
  // Phase 12: Achievements
  // ============================================================================
  {
    name: 'manage_achievements',
    category: 'utility',
    description: 'Create and configure achievements/trophies with platform integration and progress tracking.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_achievement', 'configure_platform_integration', 'setup_progress_tracking',
            'create_secret_achievements', 'configure_achievement_ui',
            'get_achievement_info', 'list_achievements'
          ],
          description: 'Achievements action to perform'
        },
        achievementId: commonSchemas.stringProp,
        achievementName: commonSchemas.stringProp,
        description: commonSchemas.stringProp,
        icon: commonSchemas.texturePath,
        points: commonSchemas.integerProp,
        category: { type: 'string', enum: ['story', 'exploration', 'combat', 'collection', 'challenge', 'multiplayer'], description: 'Achievement category.' },
        criteriaType: { type: 'string', enum: ['kill', 'collect', 'reach', 'complete', 'discover', 'custom'], description: 'Criteria type.' },
        criteriaTarget: commonSchemas.integerProp,
        isSecret: commonSchemas.booleanProp,
        platform: { type: 'string', enum: ['steam', 'psn', 'xbox', 'epic', 'all'], description: 'Platform integration.' },
        showProgress: commonSchemas.booleanProp,
        popupDuration: commonSchemas.numberProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        achievementPath: commonSchemas.assetPath,
        achievementCount: commonSchemas.integerProp
      }
    }
  },
  // ============================================================================
  // Phase 12: Analytics
  // ============================================================================
  {
    name: 'manage_analytics',
    category: 'utility',
    description: 'Configure game analytics: event tracking, funnel analysis, heatmaps, and A/B testing.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'setup_event_tracking', 'configure_funnel_analysis', 'create_heatmaps',
            'setup_session_tracking', 'configure_ab_testing',
            'get_analytics_info', 'list_events'
          ],
          description: 'Analytics action to perform'
        },
        eventName: commonSchemas.stringProp,
        eventCategory: { type: 'string', enum: ['gameplay', 'economy', 'progression', 'social', 'ui', 'technical'], description: 'Event category.' },
        eventParameters: commonSchemas.objectProp,
        funnelName: commonSchemas.stringProp,
        funnelSteps: commonSchemas.arrayOfStrings,
        heatmapType: { type: 'string', enum: ['death', 'combat', 'exploration', 'interaction'], description: 'Heatmap type.' },
        heatmapResolution: commonSchemas.numberProp,
        sessionMetrics: { type: 'array', items: { type: 'string', enum: ['duration', 'actions', 'deaths', 'progress'] }, description: 'Session metrics.' },
        testName: commonSchemas.stringProp,
        testVariants: commonSchemas.arrayOfStrings,
        testAllocation: commonSchemas.objectProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        eventId: commonSchemas.stringProp,
        testId: commonSchemas.stringProp
      }
    }
  },

  // ============================================================================
  // PHASE 15: Level & Data Tools
  // ============================================================================

  {
    name: 'manage_triggers',
    category: 'world',
    description: 'Trigger volumes and event system management. Create box/sphere/capsule triggers, configure overlap events, setup gameplay tags, and create gameplay cues.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_trigger_volume', 'configure_trigger_events', 'setup_event_dispatchers',
            'create_gameplay_tags', 'configure_gameplay_cues', 'setup_game_events',
            'add_trigger_condition', 'remove_trigger', 'list_triggers', 'get_trigger_info'
          ]
        },
        triggerType: { type: 'string', enum: ['box', 'sphere', 'capsule', 'custom'] },
        triggerId: commonSchemas.stringProp,
        triggerName: commonSchemas.stringProp,
        location: commonSchemas.location,
        size: commonSchemas.vector3,
        radius: commonSchemas.numberProp,
        eventType: { type: 'string', enum: ['overlap_begin', 'overlap_end', 'hit', 'custom'] },
        gameplayTags: commonSchemas.arrayOfStrings,
        eventName: commonSchemas.stringProp,
        conditions: commonSchemas.arrayOfStrings,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        triggerId: commonSchemas.stringProp,
        triggers: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_spawning',
    category: 'world',
    description: 'Entity spawning and object pooling system. Create spawn points, configure spawn rules, setup wave spawners, and manage object pools.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_spawn_point', 'configure_spawn_rules', 'setup_wave_spawner',
            'create_pool_system', 'configure_respawn', 'add_spawn_condition',
            'set_spawn_cooldown', 'set_spawn_limit', 'list_spawn_points', 'get_spawner_info'
          ]
        },
        spawnerId: commonSchemas.stringProp,
        spawnerName: commonSchemas.stringProp,
        location: commonSchemas.location,
        rotation: commonSchemas.rotation,
        actorClass: commonSchemas.stringProp,
        spawnConditions: commonSchemas.arrayOfStrings,
        cooldown: commonSchemas.numberProp,
        maxActive: commonSchemas.numberProp,
        waveConfig: commonSchemas.objectProp,
        poolSize: commonSchemas.numberProp,
        preload: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        spawnerId: commonSchemas.stringProp,
        spawnPoints: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_data_tables',
    category: 'utility',
    description: 'Data table and curve management. Create data tables, add/modify rows, setup curve tables, and import/export CSV data.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_data_table', 'add_data_row', 'update_data_row', 'remove_data_row',
            'configure_curve_table', 'setup_curve_asset', 'create_composite_table',
            'import_csv', 'export_csv', 'list_tables', 'get_table_info', 'query_rows'
          ]
        },
        tablePath: commonSchemas.assetPath,
        tableName: commonSchemas.stringProp,
        rowStruct: commonSchemas.stringProp,
        rowName: commonSchemas.stringProp,
        rowData: commonSchemas.objectProp,
        curveType: { type: 'string', enum: ['float', 'vector', 'color', 'custom'] },
        csvPath: commonSchemas.stringProp,
        queryFilter: commonSchemas.objectProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        tablePath: commonSchemas.stringProp,
        rows: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_data_assets',
    category: 'utility',
    description: 'Primary data asset management. Create custom data asset types, configure asset manager, setup asset registry with tags and metadata.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_data_asset', 'configure_asset_manager', 'setup_asset_registry',
            'create_soft_references', 'set_asset_tags', 'get_asset_metadata',
            'set_loading_priority', 'create_asset_bundle', 'list_assets', 'search_by_tag'
          ]
        },
        assetPath: commonSchemas.assetPath,
        assetType: commonSchemas.stringProp,
        assetName: commonSchemas.stringProp,
        parentClass: commonSchemas.stringProp,
        properties: commonSchemas.objectProp,
        tags: commonSchemas.arrayOfStrings,
        metadata: commonSchemas.objectProp,
        loadingPriority: commonSchemas.numberProp,
        bundleName: commonSchemas.stringProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        assetPath: commonSchemas.stringProp,
        assets: { type: 'array' }
      }
    }
  },

  // ============================================================================
  // PHASE 13: Environment Systems
  // ============================================================================

  {
    name: 'manage_time_weather',
    category: 'world',
    description: 'Day/night cycle and weather system management. Create time of day systems, configure weather effects, and setup seasonal changes.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_day_night_cycle', 'configure_time_speed', 'setup_weather_system',
            'create_weather_transitions', 'configure_indoor_outdoor', 'setup_seasonal_changes',
            'set_current_time', 'set_weather_state', 'add_weather_zone', 'get_time_info'
          ]
        },
        cycleId: commonSchemas.stringProp,
        dayLengthMinutes: commonSchemas.numberProp,
        startTime: commonSchemas.numberProp,
        timeScale: commonSchemas.numberProp,
        weatherType: { type: 'string', enum: ['clear', 'cloudy', 'rain', 'snow', 'fog', 'storm', 'sandstorm', 'custom'] },
        weatherIntensity: commonSchemas.numberProp,
        transitionDuration: commonSchemas.numberProp,
        season: { type: 'string', enum: ['spring', 'summer', 'fall', 'winter'] },
        indoorVolumes: commonSchemas.arrayOfStrings,
        sunSettings: commonSchemas.objectProp,
        skySettings: commonSchemas.objectProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        cycleId: commonSchemas.stringProp,
        currentTime: commonSchemas.numberProp,
        currentWeather: commonSchemas.stringProp
      }
    }
  },

  {
    name: 'manage_ecosystem',
    category: 'world',
    description: 'Wildlife and ecosystem management. Create animal species, configure herd behavior, setup predator-prey dynamics, and manage taming systems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_animal_species', 'configure_herd_behavior', 'setup_predator_prey',
            'create_spawning_system', 'configure_animal_ai', 'setup_taming_system',
            'add_habitat_zone', 'configure_migration', 'list_species', 'get_ecosystem_info'
          ]
        },
        speciesId: commonSchemas.stringProp,
        speciesName: commonSchemas.stringProp,
        behaviorType: { type: 'string', enum: ['passive', 'neutral', 'aggressive', 'predator', 'prey', 'herd'] },
        diet: { type: 'string', enum: ['herbivore', 'carnivore', 'omnivore'] },
        habitat: { type: 'string', enum: ['forest', 'plains', 'mountain', 'desert', 'water', 'cave', 'any'] },
        herdSize: commonSchemas.numberProp,
        spawnChance: commonSchemas.numberProp,
        aiProfile: commonSchemas.objectProp,
        tamingDifficulty: commonSchemas.numberProp,
        canRide: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        speciesId: commonSchemas.stringProp,
        species: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_destruction',
    category: 'world',
    description: 'Destructible environment management. Create destructible buildings, configure debris systems, setup repair mechanics, and manage environmental hazards.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_destructible_building', 'configure_debris_system', 'setup_repair_system',
            'create_environmental_hazards', 'configure_structural_integrity', 'add_damage_level',
            'set_destruction_threshold', 'enable_fire_spread', 'list_destructibles', 'get_damage_info'
          ]
        },
        destructibleId: commonSchemas.stringProp,
        destructibleName: commonSchemas.stringProp,
        meshPath: commonSchemas.meshPath,
        maxHealth: commonSchemas.numberProp,
        damageLevels: commonSchemas.arrayOfStrings,
        debrisLifetime: commonSchemas.numberProp,
        debrisCount: commonSchemas.numberProp,
        canRepair: commonSchemas.booleanProp,
        repairCost: commonSchemas.numberProp,
        fireSpreadRate: commonSchemas.numberProp,
        structuralSupports: commonSchemas.arrayOfStrings,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        destructibleId: commonSchemas.stringProp,
        currentHealth: commonSchemas.numberProp
      }
    }
  },

  {
    name: 'manage_npc_systems',
    category: 'world',
    description: 'NPC behavior and schedule management. Create NPC schedules, configure relationships, setup conversations, and manage crowd simulation.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_npc_schedule', 'configure_npc_relationships', 'setup_npc_conversation',
            'create_npc_memory', 'configure_crowd_simulation', 'add_schedule_entry',
            'set_relationship', 'add_memory_event', 'list_npcs', 'get_npc_info'
          ]
        },
        npcId: commonSchemas.stringProp,
        npcName: commonSchemas.stringProp,
        scheduleEntries: { type: 'array', items: { type: 'object' } },
        relationships: { type: 'array', items: { type: 'object' } },
        conversationTopics: commonSchemas.arrayOfStrings,
        memoryDuration: commonSchemas.numberProp,
        crowdBehavior: { type: 'string', enum: ['wander', 'gather', 'flee', 'follow', 'custom'] },
        homeLocation: commonSchemas.location,
        workLocation: commonSchemas.location,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        npcId: commonSchemas.stringProp,
        npcs: { type: 'array' }
      }
    }
  },

  // ============================================================================
  // PHASE 14: Character Systems
  // ============================================================================

  {
    name: 'manage_character_creator',
    category: 'gameplay',
    description: 'Character customization system. Create body morph systems, configure face customization, setup hair/skin/clothing systems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_body_morph_system', 'configure_face_customization', 'setup_hair_system',
            'create_skin_system', 'configure_clothing_system', 'setup_accessory_system',
            'add_preset', 'randomize_character', 'save_character', 'load_character'
          ]
        },
        characterId: commonSchemas.stringProp,
        bodyMorphs: commonSchemas.objectProp,
        faceMorphs: commonSchemas.objectProp,
        hairStyle: commonSchemas.stringProp,
        hairColor: commonSchemas.color,
        skinTone: commonSchemas.stringProp,
        skinTexture: commonSchemas.stringProp,
        clothingSlots: commonSchemas.objectProp,
        accessories: commonSchemas.arrayOfStrings,
        presetName: commonSchemas.stringProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        characterId: commonSchemas.stringProp,
        presets: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_class_system',
    category: 'gameplay',
    description: 'Character class and role management. Create classes, configure subclasses, setup multiclass rules, and define class-specific abilities.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_class', 'configure_subclass', 'setup_multiclass', 'create_class_quests',
            'configure_role_bonuses', 'add_class_ability', 'set_equipment_restrictions',
            'define_stat_growth', 'list_classes', 'get_class_info'
          ]
        },
        classId: commonSchemas.stringProp,
        className: commonSchemas.stringProp,
        classDescription: commonSchemas.stringProp,
        role: { type: 'string', enum: ['tank', 'healer', 'dps', 'support', 'hybrid'] },
        baseStats: commonSchemas.objectProp,
        statGrowth: commonSchemas.objectProp,
        startingAbilities: commonSchemas.arrayOfStrings,
        equipmentRestrictions: commonSchemas.objectProp,
        subclasses: commonSchemas.arrayOfStrings,
        multiclassRequirements: commonSchemas.objectProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        classId: commonSchemas.stringProp,
        classes: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_mount_system',
    category: 'gameplay',
    description: 'Mount and vehicle system for RPGs. Create mounts, configure abilities, setup customization, and enable mounted combat.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_mount', 'configure_mount_abilities', 'setup_mount_customization',
            'create_mount_combat', 'configure_mount_storage', 'add_mount_animation',
            'set_mount_stats', 'enable_flying', 'enable_swimming', 'list_mounts'
          ]
        },
        mountId: commonSchemas.stringProp,
        mountName: commonSchemas.stringProp,
        mountType: { type: 'string', enum: ['horse', 'wolf', 'dragon', 'mechanical', 'flying', 'aquatic', 'custom'] },
        baseSpeed: commonSchemas.numberProp,
        canFly: commonSchemas.booleanProp,
        canSwim: commonSchemas.booleanProp,
        abilities: commonSchemas.arrayOfStrings,
        customizationSlots: commonSchemas.objectProp,
        storageSlots: commonSchemas.numberProp,
        combatEnabled: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        mountId: commonSchemas.stringProp,
        mounts: { type: 'array' }
      }
    }
  },

  // ============================================================================
  // PHASE 17: Build & Platform
  // ============================================================================

  {
    name: 'manage_testing',
    category: 'utility',
    description: 'Automated testing system. Create functional tests, configure test automation, setup test harnesses, and run test suites.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_functional_test', 'configure_test_automation', 'setup_test_harness',
            'create_latent_test', 'run_test_suite', 'get_test_results', 'create_mock_data',
            'add_assertion', 'list_tests', 'clear_test_results'
          ]
        },
        testId: commonSchemas.stringProp,
        testName: commonSchemas.stringProp,
        testCategory: commonSchemas.stringProp,
        testClass: commonSchemas.stringProp,
        assertions: commonSchemas.arrayOfStrings,
        mockData: commonSchemas.objectProp,
        timeout: commonSchemas.numberProp,
        runParallel: commonSchemas.booleanProp,
        categoryFilter: commonSchemas.stringProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        testId: commonSchemas.stringProp,
        passed: commonSchemas.booleanProp,
        results: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_validation',
    category: 'utility',
    description: 'Content validation system. Validate assets, check blueprint errors, analyze performance, and verify data integrity.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'validate_assets', 'check_blueprint_errors', 'analyze_performance',
            'validate_data_integrity', 'check_naming_conventions', 'find_missing_references',
            'validate_maps', 'check_collision_settings', 'get_validation_report'
          ]
        },
        assetPaths: commonSchemas.arrayOfStrings,
        blueprintPath: commonSchemas.assetPath,
        mapPath: commonSchemas.assetPath,
        validationRules: commonSchemas.objectProp,
        namingConvention: commonSchemas.stringProp,
        severity: { type: 'string', enum: ['error', 'warning', 'info', 'all'] },
        autoFix: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        issues: { type: 'array' },
        summary: commonSchemas.objectProp
      }
    }
  },

  {
    name: 'manage_build',
    category: 'utility',
    description: 'Build system management. Configure build settings, setup build targets, create build profiles, and manage pak files.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'configure_build_settings', 'setup_build_target', 'create_build_profile',
            'configure_pak_files', 'setup_dlc_content', 'start_build', 'get_build_status',
            'cancel_build', 'list_build_profiles', 'validate_build_config'
          ]
        },
        profileName: commonSchemas.stringProp,
        targetPlatform: { type: 'string', enum: ['Windows', 'Mac', 'Linux', 'PS5', 'XboxSeriesX', 'Switch', 'iOS', 'Android'] },
        buildConfig: { type: 'string', enum: ['Debug', 'Development', 'Test', 'Shipping'] },
        pakSettings: commonSchemas.objectProp,
        compressionType: { type: 'string', enum: ['None', 'Zlib', 'Gzip', 'Oodle'] },
        encryption: commonSchemas.booleanProp,
        dlcName: commonSchemas.stringProp,
        outputDirectory: commonSchemas.stringProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        buildId: commonSchemas.stringProp,
        status: commonSchemas.stringProp,
        profiles: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_platforms',
    category: 'utility',
    description: 'Platform configuration management. Configure platform-specific settings, setup input mapping, and manage cross-platform features.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'configure_platform_settings', 'setup_platform_input', 'create_platform_overrides',
            'configure_cross_platform', 'set_scalability_settings', 'configure_platform_features',
            'list_platforms', 'get_platform_info', 'validate_platform_config'
          ]
        },
        platform: { type: 'string', enum: ['Windows', 'Mac', 'Linux', 'PS5', 'XboxSeriesX', 'Switch', 'iOS', 'Android', 'All'] },
        inputMappings: commonSchemas.objectProp,
        scalabilitySettings: commonSchemas.objectProp,
        graphicsSettings: commonSchemas.objectProp,
        audioSettings: commonSchemas.objectProp,
        crossPlayEnabled: commonSchemas.booleanProp,
        crossSaveEnabled: commonSchemas.booleanProp,
        featureFlags: commonSchemas.objectProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        platform: commonSchemas.stringProp,
        platforms: { type: 'array' }
      }
    }
  },

  // ============================================================================
  // PHASE 18-21: Runtime Systems
  // ============================================================================

  {
    name: 'manage_components',
    category: 'gameplay',
    description: 'Runtime actor component management. Create and configure quest tracker, dialogue, inventory, combat, interaction, status effect, equipment, progression, faction, stealth, and needs components.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_quest_tracker_component', 'create_dialogue_component', 'create_inventory_component',
            'create_combat_component', 'create_interaction_component', 'create_status_effect_component',
            'create_equipment_component', 'create_progression_component', 'create_faction_component',
            'create_stealth_component', 'create_needs_component', 'configure_component',
            'add_component_to_actor', 'remove_component', 'list_components', 'get_component_info'
          ]
        },
        componentType: {
          type: 'string',
          enum: ['quest_tracker', 'dialogue', 'inventory', 'combat', 'interaction', 'status_effect', 'equipment', 'progression', 'faction', 'stealth', 'needs']
        },
        actorPath: commonSchemas.assetPath,
        componentName: commonSchemas.stringProp,
        componentSettings: commonSchemas.objectProp,
        replicates: commonSchemas.booleanProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        componentId: commonSchemas.stringProp,
        components: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_framework',
    category: 'gameplay',
    description: 'Game framework class management. Create and configure MCP Game Mode, Game State, Player State, Player Controller, Character, Game Instance, and Save Game classes.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_game_mode', 'create_game_state', 'create_player_state',
            'create_player_controller', 'create_character', 'create_game_instance',
            'create_save_game', 'configure_framework_class', 'set_default_classes',
            'list_framework_classes', 'get_framework_info'
          ]
        },
        classType: {
          type: 'string',
          enum: ['game_mode', 'game_state', 'player_state', 'player_controller', 'character', 'game_instance', 'save_game', 'hud', 'cheat_manager']
        },
        className: commonSchemas.stringProp,
        parentClass: commonSchemas.stringProp,
        classSettings: commonSchemas.objectProp,
        defaultPawn: commonSchemas.assetPath,
        defaultHUD: commonSchemas.assetPath,
        defaultPlayerController: commonSchemas.assetPath,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        classPath: commonSchemas.stringProp,
        classes: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_subsystems',
    category: 'gameplay',
    description: 'World and game instance subsystem management. Create and configure Quest, Economy, Spawn, Weather, WorldGen, Dialogue, Audio, and Analytics subsystems.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'create_quest_subsystem', 'configure_quest_subsystem', 'register_quest_definition',
            'create_economy_subsystem', 'configure_economy_subsystem', 'register_shop',
            'create_spawn_subsystem', 'configure_spawn_subsystem', 'register_spawn_pool',
            'create_weather_subsystem', 'configure_weather_subsystem', 'register_weather_zone',
            'create_worldgen_subsystem', 'configure_worldgen_subsystem', 'register_worldgen_preset',
            'create_dialogue_subsystem', 'create_audio_subsystem', 'create_analytics_subsystem',
            'get_subsystem_status', 'list_subsystems', 'get_subsystem_info'
          ]
        },
        subsystemType: {
          type: 'string',
          enum: ['quest', 'economy', 'spawn', 'weather', 'worldgen', 'dialogue', 'audio', 'analytics']
        },
        subsystemName: commonSchemas.stringProp,
        subsystemSettings: commonSchemas.objectProp,
        registrationData: commonSchemas.objectProp,
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        subsystemId: commonSchemas.stringProp,
        subsystems: { type: 'array' }
      }
    }
  },

  {
    name: 'manage_templates',
    category: 'gameplay',
    description: 'Pre-built generation templates for common game patterns. Generate character, weapon, NPC, quest chain, dungeon, open world chunk, shop, and crafting station templates.',
    inputSchema: {
      type: 'object',
      properties: {
        action: {
          type: 'string',
          enum: [
            'generate_character_template', 'configure_character_template', 'add_character_abilities',
            'generate_weapon_template', 'configure_weapon_template', 'add_weapon_effects',
            'generate_npc_template', 'configure_npc_template', 'add_npc_behaviors',
            'generate_quest_chain_template', 'configure_quest_chain_template', 'add_quest_chain_quests',
            'generate_dungeon_template', 'configure_dungeon_template', 'add_dungeon_features',
            'generate_open_world_chunk_template', 'configure_open_world_chunk_template', 'add_chunk_features',
            'generate_shop_template', 'generate_crafting_station_template',
            'finalize_template', 'list_templates', 'get_template_info', 'validate_template'
          ]
        },
        templateType: {
          type: 'string',
          enum: ['character', 'weapon', 'npc', 'quest_chain', 'dungeon', 'open_world_chunk', 'shop', 'crafting_station']
        },
        templateName: commonSchemas.stringProp,
        templateSettings: commonSchemas.objectProp,
        outputPath: commonSchemas.assetPath,
        seed: commonSchemas.numberProp,
        complexity: { type: 'string', enum: ['simple', 'medium', 'complex', 'advanced'] },
        save: commonSchemas.save
      },
      required: ['action']
    },
    outputSchema: {
      type: 'object',
      properties: {
        ...commonSchemas.outputBase,
        templateId: commonSchemas.stringProp,
        generatedAssets: { type: 'array' },
        templates: { type: 'array' }
      }
    }
  }
];
