import { defineConfigWithVueTs, vueTsConfigs } from '@vue/eslint-config-typescript'

import pluginVitest from '@vitest/eslint-plugin'
import { globalIgnores } from 'eslint/config'
import pluginBoundaries from 'eslint-plugin-boundaries'
import pluginOxlint from 'eslint-plugin-oxlint'
import simpleImportSort from 'eslint-plugin-simple-import-sort'
import pluginVue from 'eslint-plugin-vue'

// To allow more languages other than `ts` in `.vue` files, uncomment the following lines:
// import { configureVueProject } from '@vue/eslint-config-typescript'
// configureVueProject({ scriptLangs: ['ts', 'tsx'] })
// More info at https://github.com/vuejs/eslint-config-typescript/#advanced-setup

export default defineConfigWithVueTs(
  {
    name: 'app/files-to-lint',
    files: ['**/*.{vue,ts,mts,tsx}'],
  },

  globalIgnores(['**/dist/**', '**/dist-ssr/**', '**/coverage/**']),

  ...pluginVue.configs['flat/recommended'],
  vueTsConfigs.strictTypeChecked,

  {
    ...pluginVitest.configs.recommended,
    files: ['src/**/__tests__/*'],
  },

  {
    plugins: {
      boundaries: pluginBoundaries,
      'simple-import-sort': simpleImportSort,
    },
    settings: {
      'import/resolver': {
        typescript: {
          alwaysTryTypes: true,
          project: './tsconfig.app.json',
        },
      },
      'boundaries/elements': [
        {
          type: 'module',
          pattern: 'src/modules/*',
          capture: ['name'],
        },
        {
          type: 'shared',
          pattern: 'src/shared',
        },
        {
          type: 'infrastructure',
          pattern: 'src/infrastructure/*',
        },
      ],
      'boundaries/ignore': ['src/**/__tests__/**/*'],
    },
    rules: {
      'simple-import-sort/imports': ['error', {
        groups: [
          // 1. Side effect imports (e.g., `import './setup'`)
          ['^\\u0000'],
          // 2. Vue and core ecosystem imports
          ['^vue', '^@vue', '^vue-router'],
          // 3. Third-party packages (node_modules)
          ['^@?\\w'],
          // 4. External modules and shared resources (using @/ alias)
          ['^@/'],
          // 5. Internal modules (relative paths)
          ['^\\.'],
          // 6. Style imports
          ['^.+\\.?(css|scss)$'],
        ],
      }],
      'simple-import-sort/exports': 'error',
      'boundaries/dependencies': [
        'error',
        {
          default: 'allow',
          rules: [
            {
              from: { type: 'module' },
              disallow: [
                { to: { type: 'module', captured: { name: '!{{from.captured.name}}' } } },
              ],
              message: 'Cross-module imports must only target the module\'s index.ts (public interface).',
            },
            {
              from: { type: 'module' },
              allow: [
                { to: { type: 'module', captured: { name: '!{{from.captured.name}}' }, internalPath: 'index.ts' } },
              ],
            },
            {
              from: ['module', 'infrastructure'],
              disallow: [
                { to: { type: 'shared', internalPath: '!(index.ts)' } },
              ],
              message: 'Shared imports must only target the shared/index.ts (public interface).',
            },
          ],
        },
      ],
    },

  },

  {
    rules: {
      '@typescript-eslint/no-explicit-any': 'error',
      'vue/multi-word-component-names': 'off', // Common preference for feature modules
    }
  },

  ...pluginOxlint.buildFromOxlintConfigFile('.oxlintrc.json'),
)
