<script lang="ts">
  import { Button } from "$lib/components/ui/button";
  import Badge from "$lib/components/ui/badge/badge.svelte";
  import * as Card from "$lib/components/ui/card";
  import { onMount } from "svelte";
  let clients = $state([]);
  let loading = $state(true);
  let error = $state(null);

  async function fetchClients() {
    loading = true;
    error = null;

    try {
      const response = await fetch('/api/conversationServer/getClientsActive');
      if (!response.ok) {
        throw new Error(`Server vrátil chybu: ${response.status}`);
      }

      clients = await response.json();
      console.log(clients);
    } catch (e) {
      error = e.message ?? 'Nepodařilo se připojit k C++ backendu.';
    } finally {
      loading = false;
    }
  }

  onMount(() => {
    fetchClients();
  });
</script>

<main class="flex-1 min-w-0 h-full p-6 space-y-6 overflow-y-auto">

  <header class="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-2 border-b">
    <div>
      <h1 class="text-2xl font-bold tracking-tight text-foreground">
          Currently active clients
      </h1>
    </div>

    <div class="flex items-center gap-2">
      <Button variant="outline" size="sm">
        Obnovit
      </Button>
    </div>
  </header>

  <Card.Root class="w-full shadow-sm border">
    <Card.Header class="border-b bg-muted/20 px-6 py-3">
      <div class="flex items-center justify-between">
        <Card.Title class="text-sm font-semibold">Aktivní zařízení</Card.Title>
        <span class="text-xs text-muted-foreground font-mono">C++ API /ws</span>
      </div>
    </Card.Header>

    <Card.Content class="p-6">
        {#if clients.length > 0}
        {#each clients as client}

        <div class="p-5 border rounded-xl bg-card text-card-foreground shadow-sm space-y-4 hover:border-primary/40 transition-all">

          <div class="flex flex-wrap items-center justify-between gap-2 pb-3 border-b">
            <div class="flex items-center gap-3">
              <span class="text-lg font-bold font-mono tracking-tight text-foreground">
                {client.ip}
              </span>

              <Badge variant={client.connected ? "default" : "destructive"}>
                {client.connected ? "Online" : "Offline"}
              </Badge>
            </div>

            <span class="text-xs font-mono text-muted-foreground bg-muted px-2.5 py-1 rounded-md">
              ID: {client.id}
            </span>
          </div>

          <div class="grid grid-cols-1 sm:grid-cols-3 gap-4 text-sm">

            <div>
              <span class="text-xs text-muted-foreground font-medium block mb-1">Uptime</span>
              <span class="font-mono font-semibold text-foreground">{client.uptime}</span>
            </div>

            <div>
              <span class="text-xs text-muted-foreground font-medium block mb-1">Konverzace</span>
              <span class="font-medium text-foreground">{client.history ?? 'Žádná'}</span>
            </div>

            <div>
              <span class="text-xs text-muted-foreground font-medium block mb-1">Aktivní Streamy</span>
              <div class="flex items-center gap-2">

                <Badge
                  variant={client.connectedStreams.audioStream ? "outline" : "secondary"}
                  class={client.connectedStreams.audioStream ? "border-green-500/50 text-green-600 bg-green-500/10" : "opacity-60"}
                >
                  Voice: {client.connectedStreams.audioStream ? "ON" : "OFF"}
                </Badge>

                <Badge
                  variant={client.connectedStreams.videoStream ? "outline" : "secondary"}
                  class={client.connectedStreams.videoStream ? "border-blue-500/50 text-blue-600 bg-blue-500/10" : "opacity-60"}
                >
                  Video: {client.connectedStreams.videoStream ? "ON" : "OFF"}
                </Badge>

              </div>
            </div>

          </div>

        </div>
        {/each}
        {/if}
    </Card.Content>

    <Card.Footer class="border-t bg-muted/10 px-6 py-2 flex items-center justify-between text-xs text-muted-foreground">
      <span>Status: Připojeno</span>
      <span>Automatické obnovení: 5s</span>
    </Card.Footer>
  </Card.Root>

</main>
