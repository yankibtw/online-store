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