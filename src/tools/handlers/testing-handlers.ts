/**
 * FORGE2: Testing System Handlers
 * Handles automated testing, functional tests, test harness, and latent tests
 */

import { ITools } from '../../types/tool-interfaces.js';
import type { HandlerArgs } from '../../types/handler-types.js';
import {
  Forge2Categories,
  executeForge2Action,
  createCategoryHandler,
  type ActionHandler,
  type ActionHandlerMap
} from './utils/forge2-bridge.js';

// ============================================================================
// Helper Functions - Validators
// ============================================================================

function validateRequiredString(value: unknown, fieldName: string): string {
  if (typeof value !== 'string' || value.trim() === '') {
    throw new Error(`${fieldName} is required and must be a non-empty string`);
  }
  return value;
}

function validateOptionalString(value: unknown): string | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value !== 'string') return undefined;
  return value.trim() || undefined;
}

function validateOptionalNumber(value: unknown): number | undefined {
  if (value === undefined || value === null) return undefined;
  const num = Number(value);
  return isNaN(num) ? undefined : num;
}

function validateOptionalBoolean(value: unknown): boolean | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value === 'boolean') return value;
  if (value === 'true') return true;
  if (value === 'false') return false;
  return undefined;
}

function validateOptionalArray(value: unknown): unknown[] | undefined {
  if (value === undefined || value === null) return undefined;
  if (Array.isArray(value)) return value;
  return undefined;
}

function validateOptionalObject(value: unknown): Record<string, unknown> | undefined {
  if (value === undefined || value === null) return undefined;
  if (typeof value === 'object' && !Array.isArray(value)) {
    return value as Record<string, unknown>;
  }
  return undefined;
}

// ============================================================================
// Functional Test Action Handlers
// ============================================================================

/**
 * Create functional test
 */
const handleCreateFunctionalTest: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const testName = validateRequiredString(params.test_name || params.testName, 'test_name');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'create_functional_test', {
    testName,
    testDescription: validateOptionalString(params.test_description || params.testDescription),
    testClass: validateOptionalString(params.test_class || params.testClass) ?? 'FunctionalTest',
    mapToLoad: validateOptionalString(params.map_to_load || params.mapToLoad),
    timeout: validateOptionalNumber(params.timeout) ?? 60.0,
    enabled: validateOptionalBoolean(params.enabled) ?? true
  });
};

/**
 * Add test assertion
 */
const handleAddTestAssertion: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const testName = validateRequiredString(params.test_name || params.testName, 'test_name');
  const assertionType = validateRequiredString(params.assertion_type || params.assertionType, 'assertion_type');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'add_test_assertion', {
    testName,
    assertionType,
    expectedValue: validateOptionalString(params.expected_value || params.expectedValue),
    actualExpression: validateOptionalString(params.actual_expression || params.actualExpression),
    message: validateOptionalString(params.message),
    continueOnFail: validateOptionalBoolean(params.continue_on_fail || params.continueOnFail) ?? false
  });
};

/**
 * Add test step
 */
const handleAddTestStep: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const testName = validateRequiredString(params.test_name || params.testName, 'test_name');
  const stepName = validateRequiredString(params.step_name || params.stepName, 'step_name');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'add_test_step', {
    testName,
    stepName,
    stepType: validateOptionalString(params.step_type || params.stepType) ?? 'action',
    action: validateOptionalString(params.action),
    parameters: validateOptionalObject(params.parameters),
    delayBefore: validateOptionalNumber(params.delay_before || params.delayBefore) ?? 0,
    delayAfter: validateOptionalNumber(params.delay_after || params.delayAfter) ?? 0
  });
};

// ============================================================================
// Test Automation Action Handlers
// ============================================================================

/**
 * Configure test automation
 */
const handleConfigureTestAutomation: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TESTING, 'configure_test_automation', {
    automationDriver: validateOptionalString(params.automation_driver || params.automationDriver) ?? 'Gauntlet',
    runInPIE: validateOptionalBoolean(params.run_in_pie || params.runInPIE) ?? true,
    runStandalone: validateOptionalBoolean(params.run_standalone || params.runStandalone) ?? false,
    reportFormat: validateOptionalString(params.report_format || params.reportFormat) ?? 'json',
    screenshotOnFail: validateOptionalBoolean(params.screenshot_on_fail || params.screenshotOnFail) ?? true
  });
};

/**
 * Run test suite
 */
const handleRunTestSuite: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TESTING, 'run_test_suite', {
    suiteName: validateOptionalString(params.suite_name || params.suiteName),
    testFilter: validateOptionalString(params.test_filter || params.testFilter),
    categories: validateOptionalArray(params.categories) ?? [],
    parallelExecution: validateOptionalBoolean(params.parallel_execution || params.parallelExecution) ?? false,
    stopOnFirstFail: validateOptionalBoolean(params.stop_on_first_fail || params.stopOnFirstFail) ?? false,
    iterations: validateOptionalNumber(params.iterations) ?? 1
  });
};

/**
 * Create test category
 */
const handleCreateTestCategory: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const categoryName = validateRequiredString(params.category_name || params.categoryName, 'category_name');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'create_test_category', {
    categoryName,
    description: validateOptionalString(params.description),
    parentCategory: validateOptionalString(params.parent_category || params.parentCategory),
    tests: validateOptionalArray(params.tests) ?? []
  });
};

// ============================================================================
// Test Harness Action Handlers
// ============================================================================

/**
 * Setup test harness
 */
const handleSetupTestHarness: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const harnessName = validateRequiredString(params.harness_name || params.harnessName, 'harness_name');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'setup_test_harness', {
    harnessName,
    mockDataEnabled: validateOptionalBoolean(params.mock_data_enabled || params.mockDataEnabled) ?? true,
    stubsEnabled: validateOptionalBoolean(params.stubs_enabled || params.stubsEnabled) ?? true,
    isolatedWorld: validateOptionalBoolean(params.isolated_world || params.isolatedWorld) ?? true,
    cleanupAfterTest: validateOptionalBoolean(params.cleanup_after_test || params.cleanupAfterTest) ?? true
  });
};

/**
 * Create mock data
 */
const handleCreateMockData: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const mockName = validateRequiredString(params.mock_name || params.mockName, 'mock_name');
  const dataType = validateRequiredString(params.data_type || params.dataType, 'data_type');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'create_mock_data', {
    mockName,
    dataType,
    mockData: validateOptionalObject(params.mock_data || params.mockData) ?? {},
    returnSequence: validateOptionalArray(params.return_sequence || params.returnSequence),
    randomize: validateOptionalBoolean(params.randomize) ?? false
  });
};

/**
 * Create stub
 */
const handleCreateStub: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const stubName = validateRequiredString(params.stub_name || params.stubName, 'stub_name');
  const targetFunction = validateRequiredString(params.target_function || params.targetFunction, 'target_function');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'create_stub', {
    stubName,
    targetFunction,
    returnValue: validateOptionalString(params.return_value || params.returnValue),
    callThrough: validateOptionalBoolean(params.call_through || params.callThrough) ?? false,
    recordCalls: validateOptionalBoolean(params.record_calls || params.recordCalls) ?? true
  });
};

// ============================================================================
// Latent Test Action Handlers
// ============================================================================

/**
 * Create latent test
 */
const handleCreateLatentTest: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const testName = validateRequiredString(params.test_name || params.testName, 'test_name');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'create_latent_test', {
    testName,
    testDescription: validateOptionalString(params.test_description || params.testDescription),
    waitForCondition: validateOptionalString(params.wait_for_condition || params.waitForCondition),
    timelineEvents: validateOptionalArray(params.timeline_events || params.timelineEvents) ?? [],
    maxDuration: validateOptionalNumber(params.max_duration || params.maxDuration) ?? 30.0
  });
};

/**
 * Add latent action
 */
const handleAddLatentAction: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const testName = validateRequiredString(params.test_name || params.testName, 'test_name');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'add_latent_action', {
    testName,
    actionType: validateOptionalString(params.action_type || params.actionType) ?? 'wait',
    waitTime: validateOptionalNumber(params.wait_time || params.waitTime) ?? 1.0,
    condition: validateOptionalString(params.condition),
    timeout: validateOptionalNumber(params.timeout) ?? 10.0,
    onTimeout: validateOptionalString(params.on_timeout || params.onTimeout) ?? 'fail'
  });
};

// ============================================================================
// Performance Testing Action Handlers
// ============================================================================

/**
 * Create performance test
 */
const handleCreatePerformanceTest: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;
  const testName = validateRequiredString(params.test_name || params.testName, 'test_name');

  return executeForge2Action(tools, Forge2Categories.TESTING, 'create_performance_test', {
    testName,
    testDescription: validateOptionalString(params.test_description || params.testDescription),
    targetFPS: validateOptionalNumber(params.target_fps || params.targetFPS) ?? 60,
    memoryThreshold: validateOptionalNumber(params.memory_threshold || params.memoryThreshold),
    cpuThreshold: validateOptionalNumber(params.cpu_threshold || params.cpuThreshold),
    gpuThreshold: validateOptionalNumber(params.gpu_threshold || params.gpuThreshold),
    duration: validateOptionalNumber(params.duration) ?? 60.0
  });
};

/**
 * Configure profiling
 */
const handleConfigureProfiling: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TESTING, 'configure_profiling', {
    captureStats: validateOptionalBoolean(params.capture_stats || params.captureStats) ?? true,
    captureFrameTimes: validateOptionalBoolean(params.capture_frame_times || params.captureFrameTimes) ?? true,
    captureMemory: validateOptionalBoolean(params.capture_memory || params.captureMemory) ?? true,
    captureGPU: validateOptionalBoolean(params.capture_gpu || params.captureGPU) ?? true,
    outputFormat: validateOptionalString(params.output_format || params.outputFormat) ?? 'csv'
  });
};

// ============================================================================
// Query Action Handlers
// ============================================================================

/**
 * Get test results
 */
const handleGetTestResults: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TESTING, 'get_test_results', {
    testName: validateOptionalString(params.test_name || params.testName),
    suiteName: validateOptionalString(params.suite_name || params.suiteName),
    includePassedTests: validateOptionalBoolean(params.include_passed_tests || params.includePassedTests) ?? true,
    includeFailedTests: validateOptionalBoolean(params.include_failed_tests || params.includeFailedTests) ?? true
  });
};

/**
 * List tests
 */
const handleListTests: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TESTING, 'list_tests', {
    category: validateOptionalString(params.category),
    enabledOnly: validateOptionalBoolean(params.enabled_only || params.enabledOnly) ?? false,
    includeLatent: validateOptionalBoolean(params.include_latent || params.includeLatent) ?? true
  });
};

/**
 * Get test coverage
 */
const handleGetTestCoverage: ActionHandler = async (tools: ITools, args: HandlerArgs) => {
  const params = args as Record<string, unknown>;

  return executeForge2Action(tools, Forge2Categories.TESTING, 'get_test_coverage', {
    moduleName: validateOptionalString(params.module_name || params.moduleName),
    includeUntested: validateOptionalBoolean(params.include_untested || params.includeUntested) ?? true
  });
};

// ============================================================================
// Handler Map
// ============================================================================

export const testingHandlers: ActionHandlerMap = {
  // Functional Tests
  create_functional_test: handleCreateFunctionalTest,
  add_test_assertion: handleAddTestAssertion,
  add_test_step: handleAddTestStep,

  // Test Automation
  configure_test_automation: handleConfigureTestAutomation,
  run_test_suite: handleRunTestSuite,
  create_test_category: handleCreateTestCategory,

  // Test Harness
  setup_test_harness: handleSetupTestHarness,
  create_mock_data: handleCreateMockData,
  create_stub: handleCreateStub,

  // Latent Tests
  create_latent_test: handleCreateLatentTest,
  add_latent_action: handleAddLatentAction,

  // Performance Testing
  create_performance_test: handleCreatePerformanceTest,
  configure_profiling: handleConfigureProfiling,

  // Query
  get_test_results: handleGetTestResults,
  list_tests: handleListTests,
  get_test_coverage: handleGetTestCoverage
};

export const handleTestingTools = createCategoryHandler(Forge2Categories.TESTING, testingHandlers);

export default testingHandlers;
