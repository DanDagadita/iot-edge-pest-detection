<template>
  <router-view />
</template>

<script setup lang="ts">
import { Capacitor } from "@capacitor/core";
import { NavigationBar } from "@capgo/capacitor-navigation-bar";

defineOptions({
  name: "App",
});

if (Capacitor.isNativePlatform()) {
  void NavigationBar.setNavigationBarColor({
    color: "#004e77",
    darkButtons: false,
  });
}
</script>
