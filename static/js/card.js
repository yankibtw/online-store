function showContent(tab) {
    document.getElementById('description').style.display = tab === 'description' ? 'block' : 'none';
    document.getElementById('reviews').style.display = tab === 'reviews' ? 'block' : 'none';
    
    document.getElementById('desc-tab').classList.toggle('active', tab === 'description');
    document.getElementById('reviews-tab').classList.toggle('active', tab === 'reviews');
}

const images = ["review-test.jpg", "review-test.jpg", "review-test.jpg", "review-test.jpg", "review-test.jpg"];
  const imageContainer = document.getElementById("imageContainer");
  
  images.slice(0, 3).forEach(src => {
      let img = document.createElement("img");
      img.src = "../img/card-img/" + src;
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