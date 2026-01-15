<template>
  <q-layout view="hHh LpR lff">
    <q-header elevated class="bg-primary text-white">
      <q-toolbar>
        <q-toolbar-title>
          <div class="cursor-pointer inline-block" @click="$router.push('/')">
            <q-avatar rounded>
              <img src="/icon.png" alt="Company Logo" />
            </q-avatar>
            Pest Detector
          </div>
        </q-toolbar-title>
      </q-toolbar>
    </q-header>

    <q-page-container>
      <router-view />
    </q-page-container>
  </q-layout>
</template>

<script setup lang="ts"></script>

<style scoped></style>
