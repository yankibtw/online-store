const token = "46631e72fac07dd8e9ddeb52b3bfda16db88deed"; 
const addressInput = document.getElementById("userAdress");
const suggestionsBox = document.getElementById("suggestions");

addressInput.addEventListener("input", async function () {
  const query = this.value.trim();

  if (query.length < 3) {
    suggestionsBox.innerHTML = "";
    suggestionsBox.style.display = "none";
    return;
  }

  const response = await fetch("https://suggestions.dadata.ru/suggestions/api/4_1/rs/suggest/address", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      "Accept": "application/json",
      "Authorization": "Token " + token
    },
    body: JSON.stringify({ query })
  });

  const result = await response.json();
  suggestionsBox.innerHTML = "";

  if (result.suggestions.length > 0) {
    suggestionsBox.style.display = "block";
    result.suggestions.forEach(suggestion => {
      const div = document.createElement("div");
      div.classList.add("suggestion");
      div.textContent = suggestion.value;
      div.addEventListener("click", () => {
        addressInput.value = suggestion.value;
        suggestionsBox.innerHTML = "";
        suggestionsBox.style.display = "none"; 
      });
      suggestionsBox.appendChild(div);
    });
  } else {
    suggestionsBox.style.display = "none"; 
  }
});


document.addEventListener("click", (e) => {
  if (!addressInput.contains(e.target) && !suggestionsBox.contains(e.target)) {
    suggestionsBox.innerHTML = "";
    suggestionsBox.style.display = "none";
  }
});

const paymentInputs = document.querySelectorAll('input[name="payment"]');

const paymentOptions = document.querySelectorAll(".payment-option");
let selectedPayment = "cash"; 

paymentOptions.forEach(option => {
  option.addEventListener("click", () => {
    paymentOptions.forEach(opt => opt.classList.remove("active"));
    option.classList.add("active");
    selectedPayment = option.getAttribute("data-value");
    console.log("Выбран способ оплаты:", selectedPayment);
  });
});

