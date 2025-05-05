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
  const params = new URLSearchParams(window.location.search);
  const productId = params.get("id");

  if (!productId) return;

  fetch(`/api/product/${productId}`)
      .then(response => {
          if (!response.ok) throw new Error("Product not found");
          return response.json();
      })
      .then(product => {
        document.getElementById("product-name-br").textContent = product.name;
          document.getElementById("product-name").textContent = product.name;
          document.getElementById("product-brand").textContent = product.brand;
          document.getElementById("product-image").src = product.image_url;
          document.getElementById("product-price").textContent = product.price + " ₽";
          document.getElementById("product-description").textContent = product.description;
      })
      .catch(err => {
          console.error(err);
          document.body.innerHTML = "<h2>Товар не найден</h2>";
      });
});

document.addEventListener("DOMContentLoaded", function() {
  const params = new URLSearchParams(window.location.search);
  const productId = params.get("id");

  function fetchReviews(productId) {
      fetch(`/api/reviews/${productId}`) 
          .then(response => response.json())
          .then(data => {
              displayReviews(data.reviews);
          })
          .catch(error => {
              console.error('Ошибка:', error);
          });
  }

  function displayReviews(reviews) {
    const reviewsContainer = document.getElementById("reviews");
    reviewsContainer.innerHTML = '';

    if (reviews.length === 0) {
        reviewsContainer.innerHTML = '<p>Отзывов пока нет.</p>';
        return;
    }

    reviews.forEach(review => {
        const reviewElement = document.createElement('div');
        reviewElement.classList.add('review');

        const userElement = document.createElement('div');
        userElement.classList.add('user');

        const userImage = document.createElement('img');
        userImage.src = review.images[1];
        userImage.alt = "User";
        userElement.appendChild(userImage);

        const userInfoElement = document.createElement('div');
        userInfoElement.classList.add('user-info');

        const userName = document.createElement('strong');
        userName.textContent = review.userName;
        userInfoElement.appendChild(userName);

        const reviewMark = document.createElement('div');
        reviewMark.classList.add('review-mark');

        const selectedSize = document.createElement('p');
        selectedSize.textContent = `Размер: ${review.selectedSize || "Не указан"}`;
        reviewMark.appendChild(selectedSize);

        const stars = document.createElement('div');
        stars.classList.add('stars');
        stars.textContent = "★★★★★".slice(0, review.rating); 
        reviewMark.appendChild(stars);

        const reviewDate = document.createElement('p');
        reviewDate.textContent = review.reviewDate;
        reviewMark.appendChild(reviewDate);

        userInfoElement.appendChild(reviewMark);
        userElement.appendChild(userInfoElement);

        reviewElement.appendChild(userElement);

        const reviewTextElement = document.createElement('p');
        reviewTextElement.textContent = review.reviewText;
        reviewElement.appendChild(reviewTextElement);

        if (review.images && review.images.length > 0) {
            const imagesContainer = document.createElement('div');
            imagesContainer.classList.add('review-images');
            review.images.forEach(imageUrl => {
                const imgElement = document.createElement('img');
                imgElement.src = imageUrl;
                imgElement.alt = 'Изображение отзыва';
                imagesContainer.appendChild(imgElement);
            });
            reviewElement.appendChild(imagesContainer);
        }

        reviewsContainer.appendChild(reviewElement);
    });
}

  fetchReviews(productId);
});
