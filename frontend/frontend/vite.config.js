import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import tailwindcss from '@tailwindcss/vite';
import path from "path";
export default defineConfig({
  plugins: [svelte(), tailwindcss()],
  resolve: {
     alias: {
       $lib: path.resolve("./src/lib"),
     },
   },
  server: {
    proxy: {
      '/api': {
        target: 'http://127.0.0.1:8081',
        changeOrigin: true,
        rewrite: (path) => path.replace(/^\/api/, ''),
      },
      '/ws': {
        target: 'ws://localhost:8081',
        ws: true
      }
    }
  }
})
