<script setup lang="ts">
import { ref } from 'vue';
import axios from 'axios';
import { Plus, Loader2 } from 'lucide-vue-next';
import { useCards } from '../composables/useCards';
import AppNavbar from '../components/AppNavbar.vue';
import CardList from '../components/CardList.vue';
import AddCardModal from '../components/AddCardModal.vue';
import type { Card } from '../types';

const { cards, isLoading, isCreating, isUpdating, createCard, updateCard, deleteCard } = useCards();

const showModal = ref(false);
const editingCard = ref<Card | null>(null);

const handleAdd = () => {
  editingCard.value = null;
  showModal.value = true;
};

const handleEdit = (card: Card) => {
  editingCard.value = card;
  showModal.value = true;
};

const handleToggle = async (card: Card) => {
  await updateCard({ uid: card.uid, data: { is_active: !card.is_active } });
};

const handleDelete = async (uid: string) => {
  if (confirm('Are you sure you want to delete this card?')) {
    await deleteCard(uid);
  }
};

const handleSubmit = async (data: Partial<Card>) => {
  try {
    if (editingCard.value) {
      await updateCard({ uid: editingCard.value.uid, data });
    } else {
      await createCard(data);
    }
    showModal.value = false;
  } catch (error: unknown) {
    if (axios.isAxiosError(error)) {
      alert(error.response?.data?.detail || 'An error occurred');
    } else {
      alert('An unexpected error occurred');
    }
  }
};
</script>

<template>
  <div class="min-h-screen pb-12">
    <AppNavbar />

    <main class="max-w-7xl mx-auto px-4">
      <div class="flex justify-between items-center mb-8">
        <div>
          <h1 class="text-2xl font-bold text-gray-900">Card Management</h1>
          <p class="text-gray-500">Manage RFID cards and access permissions</p>
        </div>
        <button @click="handleAdd" 
                class="flex items-center px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 transition-colors shadow-sm">
          <Plus class="w-5 h-5 mr-2" />
          Add Card
        </button>
      </div>

      <div v-if="isLoading" class="flex flex-col items-center justify-center py-24 text-gray-400">
        <Loader2 class="w-12 h-12 animate-spin mb-4" />
        <p>Loading cards...</p>
      </div>

      <template v-else>
        <CardList v-if="cards && cards.length > 0" 
                  :cards="cards" 
                  @edit="handleEdit" 
                  @delete="handleDelete"
                  @toggle="handleToggle" />
        
        <div v-else class="bg-white rounded-2xl border border-dashed border-gray-200 p-16 text-center">
          <div class="bg-gray-50 w-16 h-16 rounded-full flex items-center justify-center mx-auto mb-4">
            <Plus class="w-8 h-8 text-gray-400" />
          </div>
          <h3 class="text-lg font-medium text-gray-900 mb-1">No cards registered</h3>
          <p class="text-gray-500 mb-6">Register your first RFID card to start managing access.</p>
          <button @click="handleAdd" class="inline-flex items-center text-blue-600 font-medium hover:underline">
            Register your first card
          </button>
        </div>
      </template>
    </main>

    <AddCardModal :show="showModal" 
                  :editing-card="editingCard" 
                  :loading="isCreating || isUpdating"
                  @close="showModal = false" 
                  @submit="handleSubmit" />
  </div>
</template>
