let count = 1;
const countElement = document.querySelector(".count");
const pricePerItem = parseFloat(document.querySelector(".product-cost").textContent);
const priceSelector = document.querySelector(".product-cost");

function increment(event) {
    event.preventDefault();
    count++;
    countElement.textContent = count;
    updatePrice();
}

function decrement(event) {
    event.preventDefault();
    if (count > 1) {
        count--;
        countElement.textContent = count;
        updatePrice();
    }
}

function updatePrice(){
    priceSelector.textContent = count * pricePerItem  + "₽";
}