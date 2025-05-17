document.addEventListener("DOMContentLoaded", () => {

  /********************************************************************************/
  /**********************************frontend**************************************/
  /********************************************************************************/

  const userInput = document.querySelector('#userInput');
  const submitBtn = document.querySelector('#send-message');
  const responseArea = document.querySelector('#responseArea');
  const aiToggle = document.querySelector('#ai-toggle');
  const chatContainer = document.querySelector('#chat-container');

  userInput.addEventListener('input', function () {
    this.style.height = 'auto';
    this.style.height = Math.min(this.scrollHeight, 100) + 'px';
  });

  aiToggle.addEventListener('click', function () {
    document.querySelector("#layover").style.display = "flex"; 
    chatContainer.classList.add("ai-show");
  });

  /********************************************************************************/
  /***************************************AI***************************************/
  /********************************************************************************/

  const DEEPSEEK_API_KEY = 'sk-46fd6d8c2e694aaeb5958d89fd6e0792';
  const API_ENDPOINT = 'https://api.deepseek.com/v1/chat/completions';

  submitBtn.addEventListener('click', async () => {
    await processQuery();
  });

  async function processQuery() {
    const question = userInput.value.trim();
    if (!question) return;

    try {
      submitBtn.disabled = true;
      responseArea.innerHTML = '<div class="loading-dots"></div>';

      const data = await handleWarehouseQuery(question);
      displayResponse(data);
    } catch (error) {
      displayError(error);
    } finally {
      submitBtn.disabled = false;
    }
  }

  async function handleWarehouseQuery(userMsg) {
    const systemPrompt = `Sei un esperto AI specializzato in magazzini automatici.
Fornisci risposte tecniche su:
- Sistemi AS/RS
- Ottimizzazione layout
- Tecnologie RFID/IoT
- Standard di sicurezza ANSI/RIA R15.08
- Software WMS avanzati

Rifiuta gentilmente richieste non pertinenti.`;

    const payload = {
      model: "deepseek-chat",
      messages: [
        { role: "system", content: systemPrompt },
        { role: "user", content: userMsg }
      ],
      temperature: 0.3,
      max_tokens: 500,
      stream: false  // Aggiunto per essere esplicito
    };

    const response = await fetch(API_ENDPOINT, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${DEEPSEEK_API_KEY}`
      },
      body: JSON.stringify(payload)
    });

    if (!response.ok) {
      throw new Error(`Errore API: ${response.status}`);
    }

    return response.json();
  }

  function displayResponse(data) {
    const content = data.choices[0].message.content;
    const sanitizedContent = content
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;');

    responseArea.innerHTML = `
    <div class="response-card">
      ${sanitizedContent.replace(/\n/g, '<br>')}
    </div>
  `;
  }

  function displayError(error) {
    console.error('Errore:', error);
    responseArea.innerHTML = `
    <div class="response-card" style="color: #dc3545">
      Errore: ${error.message || 'Richiesta fallita'}
    </div>
  `;
  }
});
