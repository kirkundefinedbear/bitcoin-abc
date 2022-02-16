<?php

/**
 * This file is automatically generated. Use 'arc liberate' to rebuild it.
 *
 * @generated
 * @phutil-library-version 2
 */
phutil_register_library_map(array(
  '__library_version__' => 2,
  'class' => array(
    'ArcanistBitcoinABCConfiguration' => 'configuration/ArcanistBitcoinABCConfiguration.php',
    'ArcanistLandBotWorkflow' => 'workflow/ArcanistLandBotWorkflow.php',
    'AssertWithSideEffectsLinter' => 'linter/AssertWithSideEffectsLinter.php',
    'AutoPEP8FormatLinter' => 'linter/AutoPEP8Linter.php',
    'BashShebangLinter' => 'linter/BashShebangLinter.php',
    'BoostDependenciesLinter' => 'linter/BoostDependenciesLinter.php',
    'CHeaderLinter' => 'linter/CHeaderLinter.php',
    'CheckDocLinter' => 'linter/CheckDocLinter.php',
    'CheckNonFatalOverAssertInRpc' => 'linter/CheckNonFatalOverAssertInRpc.php',
    'ClangFormatLinter' => 'linter/ClangFormatLinter.php',
    'CppVoidParameterLinter' => 'linter/CppVoidParameterLinter.php',
    'DoxygenLinter' => 'linter/DoxygenLinter.php',
    'ExtendedConfigurationDrivenLintEngine' => 'engine/ExtendedConfigurationDrivenLintEngine.php',
    'FileNameLinter' => 'linter/FileNameLinter.php',
    'FormatStringLinter' => 'linter/FormatStringLinter.php',
    'GlobalExternalLinter' => 'linter/GlobalExternalLinter.php',
    'ILintOnce' => 'linter/ILintOnce.php',
    'ISortFormatLinter' => 'linter/ISortLinter.php',
    'IncludeGuardLinter' => 'linter/IncludeGuardLinter.php',
    'IncludeQuotesLinter' => 'linter/IncludeQuotesLinter.php',
    'IncludeSourceLinter' => 'linter/IncludeSourceLinter.php',
    'LocaleDependenceLinter' => 'linter/LocaleDependenceLinter.php',
    'LogLinter' => 'linter/LogLinter.php',
    'MarkdownLinter' => 'linter/MarkdownLinter.php',
    'MyPyLinter' => 'linter/MyPyLinter.php',
    'PrettierLinter' => 'linter/PrettierLinter.php',
    'PythonFileEncodingLinter' => 'linter/PythonFileEncodingLinter.php',
    'PythonFormatLinter' => 'linter/PythonFormatLinter.php',
    'PythonMutableDefaultLinter' => 'linter/PythonMutableDefaultLinter.php',
    'PythonShebangLinter' => 'linter/PythonShebangLinter.php',
    'Qt5Linter' => 'linter/Qt5Linter.php',
    'ShellCheckLinter' => 'linter/ShellCheckLinter.php',
    'ShellLocaleLinter' => 'linter/ShellLocaleLinter.php',
    'StdintLinter' => 'linter/StdintLinter.php',
    'TestsLinter' => 'linter/TestsLinter.php',
    'WhitespaceLinter' => 'linter/WhitespaceLinter.php',
    'YamllintLinter' => 'linter/YamllintLinter.php',
  ),
  'function' => array(
    'startsWith' => 'linter/MarkdownLinter.php',
  ),
  'xmap' => array(
    'ArcanistBitcoinABCConfiguration' => 'ArcanistConfiguration',
    'ArcanistLandBotWorkflow' => 'ArcanistWorkflow',
    'AssertWithSideEffectsLinter' => 'ArcanistLinter',
    'AutoPEP8FormatLinter' => 'ArcanistExternalLinter',
    'BashShebangLinter' => 'ArcanistLinter',
    'BoostDependenciesLinter' => 'GlobalExternalLinter',
    'CHeaderLinter' => 'ArcanistLinter',
    'CheckDocLinter' => 'GlobalExternalLinter',
    'CheckNonFatalOverAssertInRpc' => 'ArcanistLinter',
    'ClangFormatLinter' => 'ArcanistExternalLinter',
    'CppVoidParameterLinter' => 'ArcanistLinter',
    'DoxygenLinter' => 'ArcanistLinter',
    'ExtendedConfigurationDrivenLintEngine' => 'ArcanistLintEngine',
    'FileNameLinter' => 'ArcanistLinter',
    'FormatStringLinter' => 'ArcanistExternalLinter',
    'GlobalExternalLinter' => array(
      'ArcanistExternalLinter',
      'ILintOnce',
    ),
    'ISortFormatLinter' => 'ArcanistExternalLinter',
    'IncludeGuardLinter' => 'ArcanistLinter',
    'IncludeQuotesLinter' => 'ArcanistLinter',
    'IncludeSourceLinter' => 'ArcanistLinter',
    'LocaleDependenceLinter' => 'ArcanistLinter',
    'LogLinter' => 'ArcanistLinter',
    'MarkdownLinter' => 'ArcanistLinter',
    'MyPyLinter' => 'ArcanistExternalLinter',
    'PrettierLinter' => 'ArcanistExternalLinter',
    'PythonFileEncodingLinter' => 'ArcanistLinter',
    'PythonFormatLinter' => 'ArcanistExternalLinter',
    'PythonMutableDefaultLinter' => 'ArcanistLinter',
    'PythonShebangLinter' => 'ArcanistLinter',
    'Qt5Linter' => 'ArcanistLinter',
    'ShellCheckLinter' => 'ArcanistExternalLinter',
    'ShellLocaleLinter' => 'ArcanistLinter',
    'StdintLinter' => 'ArcanistLinter',
    'TestsLinter' => 'ArcanistExternalLinter',
    'WhitespaceLinter' => 'ArcanistLinter',
    'YamllintLinter' => 'ArcanistExternalLinter',
  ),
));
