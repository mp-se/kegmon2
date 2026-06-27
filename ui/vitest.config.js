import { defineConfig } from 'vitest/config'
import vue from '@vitejs/plugin-vue'
import { fileURLToPath } from 'url'
import path from 'path'

const __dirname = path.dirname(fileURLToPath(import.meta.url))

export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, 'src'),
      '@mp-se/espframework-ui-components': path.resolve(__dirname, 'node_modules/espframework-ui-components/dist/index.esm.js')
    }
  },
  test: {
    globals: true,
    environment: 'jsdom',
    setupFiles: 'src/tests/setup.js',
    coverage: {
      provider: 'v8',
      include: ['src/**/*.{js,vue}'],
      exclude: [
        'src/tests/**',
        'src/**/__tests__/**',
        'src/main.js'
      ],
      reporter: ['text', 'lcov', 'html'],
      reportsDirectory: './coverage'
    }
  }
})
