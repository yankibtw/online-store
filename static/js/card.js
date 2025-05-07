function showContent(tab) {
    document.getElementById('product-description').style.display = tab === 'description' ? 'block' : 'none';
    document.getElementById('reviews').style.display = tab === 'reviews' ? 'block' : 'none';
    
    document.getElementById('desc-tab').classList.toggle('active', tab === 'description');
    document.getElementById('reviews-tab').classList.toggle('active', tab === 'reviews');
}

const images = ["review-test.jpg", "review-test.jpg", "review-test.jpg", "review-test.jpg", "review-test.jpg"];
const imageContainer = document.getElementById("imageContainer");

images.slice(0, 3).forEach(src => {
    let img = document.createElement("img");
    img.src = "../static/img/card-img/" + src;
    img.alt = "Product image";
    imageContainer.appendChild(img);
});

if (images.length > 3) {
    let morePhotos = document.createElement("div");
    morePhotos.classList.add("more-photos");
    morePhotos.textContent = `+${images.length - 3} фото`;
    imageContainer.appendChild(morePhotos);
}

const toastTrigger = document.getElementById('liveToastBtn')
const toastLiveExample = document.getElementById('liveToast')

if (toastTrigger) {
  const toastBootstrap = bootstrap.Toast.getOrCreateInstance(toastLiveExample)
  toastTrigger.addEventListener('click', () => {
    toastBootstrap.show()
  })
}

document.addEventListener("DOMContentLoaded", function () {
    const productId = new URLSearchParams(window.location.search).get("id");
    if (!productId) return;
  
    const reviewsContainer = document.getElementById("reviews");
  
    fetch(`/api/product/${productId}`)
        .then(response => {
            if (!response.ok) throw new Error("Товар не найден");
            return response.json();
        })
        .then(product => {
            renderProduct(product);

            return fetch(`/api/reviews/${productId}`);
        })
        .then(res => {
            if (!res.ok) throw new Error("Ошибка загрузки отзывов");
            return res.json();
        })
        .then(data => {
            renderReviews(data.reviews);
        })
        .catch(err => {
            console.error(err);
            document.getElementById("reviews").innerHTML = `<div class="add-review">
                  <p>У данного товара нет отзывов. Станьте первым, кто оставил отзыв об этом товаре!</p>
                  <button>Написать отзыв</button>
                </div>`;
        });
  
    function showMessage(msg) {
        reviewsContainer.innerHTML = `<p>${msg}</p>`;
    }
  
    function renderReviews(reviews = []) {
      if (!Array.isArray(reviews) || reviews.length === 0) {
        
        return `<div class="add-review">
                  <p>У данного товара нет отзывов. Станьте первым, кто оставил отзыв об этом товаре!</p>
                  <button>Написать отзыв</button>
                </div>`;
      }
  
      reviewsContainer.innerHTML = reviews.map(renderReview).join('');
    }
  
    function renderReview({ userName, reviewText, selectedSize, rating, reviewDate, images = [] }) {
      const avatar = images[0] || "../static/img/card-img/review.png";
      const stars = "★".repeat(rating);
  
      const imageBlock = images.length
        ? `<div class="images">
              ${images.map(url => `<img src="${url}" alt="Изображение отзыва">`).join('')}
           </div>`
        : '';
  
      return `
        <div class="review">
          <div class="user">
            <img src="${avatar}" alt="User">
            <div class="user-info">
              <strong>${userName}</strong>
              <div class="review-mark">
                <p>Размер: ${selectedSize || "Не указан"}</p>
                <div style="display: flex; gap: 30px">
                  <div class="stars">${stars}</div>
                  <p>${reviewDate}</p>
                </div>
              </div>
            </div>
          </div>
          <p>${reviewText}</p>
          ${imageBlock}
        </div>
      `;
    }
  
    function renderProduct(product) {
      document.getElementById("product-name-br").textContent = product.name;
      document.getElementById("product-name").textContent = product.name;
      document.getElementById("product-brand").textContent = product.brand;
      document.getElementById("sku-txt").textContent = product.sku;
      document.getElementById("product-image").src = product.image_url;
      document.getElementById("product-price").textContent = (product.price - product.discount_price) + " ₽";
      document.getElementById("product-disc-price").textContent = product.price + " ₽";
      document.getElementById("product-description").textContent = product.description;
    }

    document.querySelectorAll(".add-to-favourite").forEach(button => {
        button.addEventListener("click", async function () {

            try {
                const response = await fetch(`/api/favorites/add/${productId}`, {
                    method: "POST",
                    credentials: "include",
                });

                const result = await response.json();

                if (response.ok) {
                    alert("Товар добавлен в избранное!");
                } else {
                    alert("Войдите в аккаунт, чтобы добавить товар в избранное!");
                }
            } catch (err) {
                alert("Ошибка сети");
                console.error(err);
            }
        });
    });
    
    const addToCartBtn = document.getElementById("add-to-cart-btn");

    if (addToCartBtn) {
        addToCartBtn.addEventListener("click", async function () {
            const productId = new URLSearchParams(window.location.search).get("id");
            const selectedSize = document.querySelector('input[name="size_option"]:checked');
    
            if (!selectedSize) {
                alert("Пожалуйста, выберите размер перед добавлением в корзину.");
                return;
            }
    
            try {
                const response = await fetch(`/api/cart/add/${productId}`, {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json",
                    },
                    credentials: "include",
                    body: JSON.stringify({ size: selectedSize.value })
                });
    
                const result = await response.json();
    
                if (response.ok) {
                    alert(result.message || "Товар успешно добавлен в корзину!");
                } else {
                    alert(result.error || "Не удалось добавить товар в корзину.");
                }
            } catch (err) {
                console.error(err);
                alert("Ошибка при добавлении в корзину.");
            }
        });
    }
    
});
  