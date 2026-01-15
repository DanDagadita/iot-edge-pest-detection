<template>
  <div
    class="fullscreen bg-primary text-white text-center q-pa-md flex flex-center"
  >
    <div>
      <div style="font-size: 20vh">404</div>

      <div class="text-h2" style="opacity: 0.4">Nothing here...</div>

      <q-btn
        class="q-mt-xl"
        color="white"
        text-color="primary"
        unelevated
        label="Back"
        no-caps
        @click="goBack"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import { useRouter } from "vue-router";

const router = useRouter();

async function goBack() {
  if (router.options.history.state.back) {
    router.go(-1);
  } else {
    await router.push("/");
  }
}
</script>
