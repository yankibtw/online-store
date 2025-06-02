document.addEventListener("DOMContentLoaded", function () {
  const stars = document.querySelectorAll("#starRating .star");
  const ratingInput = document.getElementById("ratingValue");
  const previewContainer = document.getElementById("preview");
  const cloudName = "dhbj8sdvb";
  const uploadPreset = "store-online";
    const urlParams = new URLSearchParams(window.location.search);
    const productId = urlParams.get('id');  

  stars.forEach((star, index) => {
    star.addEventListener("click", () => {
      const value = parseInt(star.dataset.value);
      ratingInput.value = value;
      stars.forEach((s, i) => {
        s.classList.toggle("selected", i < value);
      });
    });

    star.addEventListener("mouseover", () => {
      stars.forEach((s, i) => {
        s.classList.toggle("hovered", i <= index);
      });
    });

    star.addEventListener("mouseout", () => {
      stars.forEach(s => s.classList.remove("hovered"));
    });
  });

  document.getElementById("photoUpload").addEventListener("change", function () {
    previewContainer.innerHTML = "";
    Array.from(this.files).forEach(file => {
      const reader = new FileReader();
      reader.onload = e => {
        const img = document.createElement("img");
        img.src = e.target.result;
        img.style.maxWidth = "80px";
        img.style.maxHeight = "80px";
        img.classList.add("rounded");
        previewContainer.appendChild(img);
      };
      reader.readAsDataURL(file);
    });
  });

  async function uploadToCloudinary(file) {
    const url = `https://api.cloudinary.com/v1_1/${cloudName}/upload`;
    const formData = new FormData();
    formData.append("file", file);
    formData.append("upload_preset", uploadPreset);

    const res = await fetch(url, {
      method: "POST",
      body: formData
    });

    const data = await res.json();
    return data.secure_url;
  }


  document.getElementById("reviewForm").addEventListener("submit", async function (e) {
    e.preventDefault();

    const rating = ratingInput.value;
    const size = document.getElementById("sizeSelect").value;
    const comment = document.getElementById("reviewText").value;
    const files = document.getElementById("photoUpload").files;
    const ratingError = document.getElementById("ratingError");
    const sizeError = document.getElementById("sizeError");

    let valid = true;

    if (!rating) {
      ratingError.classList.remove("d-none");
      valid = false;
    } else {
      ratingError.classList.add("d-none");
    }

    if (!size) {
      sizeError.classList.remove("d-none");
      valid = false;
    } else {
      sizeError.classList.add("d-none");
    }

    if (!valid) return;

    const image_urls = [];
    for (const file of files) {
      try {
        const url = await uploadToCloudinary(file);
        image_urls.push(url);
      } catch (err) {
        alert("Ошибка при загрузке изображения. Попробуйте другое фото.");
        return;
      }
    }

    try {
      const response = await fetch(`/api/reviews/add/${productId}`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        credentials: "include",
        body: JSON.stringify({
          rating: parseInt(rating),
          comment,
          selected_size: size,
          image_urls
        })
      });

      const result = await response.json();

      if (response.ok) {
        alert("Отзыв отправлен!");
        const modal = bootstrap.Modal.getInstance(document.getElementById('reviewModal'));
        modal.hide();
        this.reset();
        previewContainer.innerHTML = "";
        stars.forEach(s => s.classList.remove("selected"));
        
      } else {
        alert("Ошибка: " + (result.error || "не удалось отправить отзыв."));
      }
    } catch (err) {
      alert("Сетевая ошибка при отправке отзыва.");
      console.error(err);
    }
  });
});
