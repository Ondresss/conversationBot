<script>
  import { Button } from "$lib/components/ui/button";
  import * as Card from "$lib/components/ui/card";
  import { Switch } from "$lib/components/ui/switch";
  import { Badge } from "$lib/components/ui/badge";

</script>

<section>
      <Card.Root class="h-full w-64 shrink-0 shadow-md">
        <Card.Header class="flex flex-row items-center justify-between pb-2">
          <Card.Title class="text-xl">Web toy controller</Card.Title>

        </Card.Header>

        <Card.Description class="px-6">
            You can control the toy and its behaviour from the web interface.
        </Card.Description>

        <Card.Content class="space-y-4">
            <div class="space-y-1">
                <h3 class="text-sm font-medium text-foreground">Actions</h3>
            </div>

            <div class="flex flex-col gap-2">
                <Button
                    variant="outline"
                    class="w-full justify-start"
                    onclick={() => alert('Kliknuto!')}
                >
                    Show connected clients
                </Button>
            </div>
        </Card.Content>

      </Card.Root>
</section>
