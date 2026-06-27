#!/bin/bash

# Used to copy shared components and their tests across projects
# This includes views, fragments, and their corresponding test files

# Fragments - components
cp ../../gravitymon-ui/src/fragments/AdvancedFilesFragment.vue ./src/fragments/AdvancedFilesFragment.vue
cp ../../gravitymon-ui/src/fragments/ListFilesFragment.vue ./src/fragments/ListFilesFragment.vue
cp ../../gravitymon-ui/src/fragments/VoltageFragment.vue ./src/fragments/VoltageFragment.vue
cp ../../gravitymon-ui/src/fragments/EnableCorsFragment.vue ./src/fragments/EnableCorsFragment.vue

# Fragments - tests
cp ../../gravitymon-ui/src/fragments/__tests__/AdvancedFilesFragment.test.js ./src/fragments/__tests__/AdvancedFilesFragment.test.js
cp ../../gravitymon-ui/src/fragments/__tests__/ListFilesFragment.test.js ./src/fragments/__tests__/ListFilesFragment.test.js
cp ../../gravitymon-ui/src/fragments/__tests__/VoltageFragment.test.js ./src/fragments/__tests__/VoltageFragment.test.js
cp ../../gravitymon-ui/src/fragments/__tests__/EnableCorsFragment.test.js ./src/fragments/__tests__/EnableCorsFragment.test.js

# Views - components
cp ../../gravitymon-ui/src/views/FirmwareView.vue ./src/views/FirmwareView.vue
cp ../../gravitymon-ui/src/views/SerialView.vue ./src/views/SerialView.vue
cp ../../gravitymon-ui/src/views/NotFoundView.vue ./src/views/NotFoundView.vue
cp ../../gravitymon-ui/src/views/DeviceWifiView.vue ./src/views/DeviceWifiView.vue
cp ../../gravitymon-ui/src/views/DeviceWifi2View.vue ./src/views/DeviceWifi2View.vue
cp ../../gravitymon-ui/src/views/ToolsView.vue ./src/views/ToolsView.vue

# Views - tests
cp ../../gravitymon-ui/src/views/__tests__/FirmwareView.test.js ./src/views/__tests__/FirmwareView.test.js
cp ../../gravitymon-ui/src/views/__tests__/SerialView.test.js ./src/views/__tests__/SerialView.test.js
cp ../../gravitymon-ui/src/views/__tests__/NotFoundView.test.js ./src/views/__tests__/NotFoundView.test.js
cp ../../gravitymon-ui/src/views/__tests__/DeviceWifiView.test.js ./src/views/__tests__/DeviceWifiView.test.js
cp ../../gravitymon-ui/src/views/__tests__/DeviceWifi2View.test.js ./src/views/__tests__/DeviceWifi2View.test.js
cp ../../gravitymon-ui/src/views/__tests__/ToolsView.test.js ./src/views/__tests__/ToolsView.test.js

echo "Copy complete. Shared components and tests synced from gravitymon-ui"
